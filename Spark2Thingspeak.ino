// This #include statement was automatically added by the Spark IDE.
#include "idDHT22/idDHT22.h"

String writeAPIKey = "write API key";
String channelID = "channel ID";

bool lastConnected = false;

// Serial Logging levels, 0 = none, 1 = error, 2 = Full Debug
int logging = 2;
//for Full debug if(logging >= 2){Serial.println("");}
//for error if(logging >= 1){Serial.println("");}

// declaration for DHT22 handler
int idDHT22pin = D4; //Digital pin for comunications
void dht22_wrapper(); // must be declared before the lib initialization

// Time that the Core will go into deep sleep (in second)
int deepSleep = 60;

// DHT instantiate
idDHT22 DHT22(idDHT22pin, dht22_wrapper);

// TCP socket initialize
TCPClient client;

void setup()
{
    if(logging >= 1)
    {
        Serial.begin(9600);
        delay(10000);
        Serial.println("===========================================================================");
        Serial.println("=== Starting ===");
    }
    if(logging >= 2)
    {
        Serial.print("-Network SSID: ");
        Serial.println(Network.SSID());
        Serial.print("-Local IP: ");
        Serial.println(Network.localIP());
        Serial.println("-Link to view streamed data: https://thingspeak.com/channels/"+channelID);
    }
}

void dht22_wrapper() {
	DHT22.isrCallback();
}

void loop() 
{
    //If we are able to read from the DHT22 sensor we update Xively
    if(Spark.connected())
    {
        if(logging >= 1){Serial.println(".Connected to Spark Cloud");}
        if (readDHT())
        {
            String temp = String(DHT22.getFahrenheit(), 2);
            String humidity = String(DHT22.getHumidity(), 2);
            
            // Print Update Response to Serial Monitor
            if (client.available())
            {
                char c = client.read();
                Serial.print(c);
            }
             
            // Disconnect from ThingSpeak
            if (!client.connected() && lastConnected)
            {
                Serial.println("...disconnected");
                Serial.println();
                
                client.stop();
            }
              
            // Update ThingSpeak
            if(!client.connected())
            {
                ThingSpeakUpdate("field1="+temp+"&field2="+humidity);
            }
              
            lastConnected = client.connected();
            
            if(logging >= 1){Serial.println("===========================================================================");}
            delay(5000);
        }
    }
}


void ThingSpeakUpdate(String tsData) {
    int status;
    
    if(logging >=2)
    {
        Serial.println("-Data String: "+tsData);
        Serial.print("-Data String Length: ");
        Serial.println(tsData.length());
    }
    
    if(logging >= 1){Serial.println("...Attempting to update Thingspeak");}
    
    if(client.connect("api.thingspeak.com", 80))
    {
        if(logging >= 1){Serial.println("...Connection succesful, updating datastreams");}
        
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(tsData.length());
        client.print("\n\n");

        client.print(tsData);
        
        status = client.available();
        if(status >= 1){
            // status is a value greater than the failure
            if(logging >= 1){Serial.println(".Update Successful!");}
            if(logging >= 2)
            {
                Serial.print("-Client Read: ");
                Serial.println(client.read());
            }
        }
        else{
            // status is equal to the failure response
            if(logging >= 1){Serial.println(".Failure from Thingspeak");}
            if(logging >= 2)
            {
                Serial.print("-Client Read: ");
                Serial.println(client.read());
            }
        }
    }
    else{
        //failed to connect
        if(logging >= 1){Serial.println(".Unable to connect to Thingspeak");}
    }
    if(!client.connected()){
        client.stop();
    }
    client.flush();
    client.stop();
}

/*------------------------------------------------
Reads the DHT sensor
Inputs: none
Returns: True/False
------------------------------------------------*/
int readDHT()
{
    if(logging >= 1){Serial.print("...Attempting to read the DHT22 sensor");}
    DHT22.acquire();
	while (DHT22.acquiring())
	{
	    delay(10);
	    if(logging >= 2)
	    {
	        Serial.print(".");
	    }
	}
	if(logging >= 1){Serial.println(".Acquired");}
	
	int result = DHT22.getStatus();

    if (result == IDDHTLIB_OK)
    {
        if(logging >= 1){Serial.println(".DHT22 read successfully");}
	    if(logging >= 2)
	    {
	        Serial.print("-Temp oF: ");
	        Serial.println(DHT22.getFahrenheit());
	        Serial.print("-Humidity: ");
	        Serial.println(DHT22.getHumidity());
	    }
	    return 1;
    } else {
		if(logging >= 1){Serial.println(".Failed to read the DHT22 sensor");}
		return 0;
	}
}
