// This #include statement was automatically added by the Spark IDE.
#include "idDHT22/idDHT22.h"

/*--------------------------------------------------------
dht22thingspeak.ino
DHT22 temp/humidity sensor logging to Thingspeak

Author: Chris Crumpacker
Date: July 2014

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

Sketch Notes:

--------------------------------------------------------*/

// Thinkspeak channel information
String writeAPIKey = "4EDI484WWVFRTEYA";
String channelID = "14187";

// Version numbering
#define VERSION 2.04

bool lastConnected = false;

// Serial Logging levels, 0 = none, 1 = error, 2 = Full Debug
int logging = 2;

// declaration for DHT22 handler
int idDHT22pin = D4; //Digital pin for comunications
void dht22_wrapper(); // must be declared before the lib initialization
int dht22PowerPin = D3;

// Time that the WiFi will go into sleep (in second)
float minutesOfSleep = 5;
int secondsOfSleep = minutesOfSleep*60; //Convert to milliseconds

//reconnect timelimits (in milliseconds)
int wifiReconnect = 20000;
int cloudReconnect = 20000;
int dhtReconnect = 10000;
int wifiTimeout;
int cloudTimeout;
int dhtTimeout;

// DHT instantiate
idDHT22 DHT22(idDHT22pin, dht22_wrapper);

// TCP socket initialize
TCPClient client;

/*--------------------------------------------------------
Setup
--------------------------------------------------------*/
void setup()
{
    //Turning the onboard RGB LED down to 10% to save battery
    RGB.control(true);
    RGB.brightness(25);
    RGB.control(false);
    
    // Supplying power to the DHT22
    pinMode(dht22PowerPin, OUTPUT);
    digitalWrite(dht22PowerPin, HIGH);
    
    // Startup information logging
    if(logging >= 1)
    {
        Serial.begin(9600);
        delay(10000);
        Serial.println("===========================================================================");
        Serial.println("=== Starting ===");
        Serial.print("Version: ");
        Serial.println(VERSION);
        Serial.println(nowDT());
    }
    if(logging >= 2)
    {
        Serial.print("-Network SSID: ");
        Serial.println(Network.SSID());
        Serial.print("-Local IP: ");
        Serial.println(Network.localIP());
        if(Spark.connected())
        {
            Serial.println("Connected to Spark Cloud.");
        }
        Serial.println("Link to view streamed data: https://thingspeak.com/channels/"+channelID);
    }
    if(logging >=1){Serial.println("===========================================================================");}
}

void dht22_wrapper() {
	DHT22.isrCallback();
}

/*--------------------------------------------------------
Main loop
--------------------------------------------------------*/
void loop() 
{
    switch ((int)WiFi.status())
    {
        case 0:
            wifiAsleep();
            break;
        case 1:
            wifiConnecting();
            break;
        case 2:
            if(logging >=1){Serial.println("WiFi Connected.");}
            if(cloudConnected())
            {
                if(logging >=1){Serial.println("Spark Cloud Connected.");}
                
                // Attempt to read the DHT22 sensor, if successful the function returns true.
                if (readDHT())
                {
                    //Set the sensor values (that are floats) to be strings
                    String tempF = String(DHT22.getFahrenheit(), 2);
                    String humidity = String(DHT22.getHumidity(), 2);
                    
                    // Update ThingSpeak
                    if(!client.connected())
                    {
                        ThingSpeakUpdate("field1="+tempF+"&field2="+humidity);
                    }
                      
                    lastConnected = client.connected();
                } 
                else // If we fail to read the DHT22 reset the power by bouncing the power pin
                {
                    if(logging >= 1){Serial.println("Failure to read the DHT22 sensor.");}
                }
            }
            
            // Put the Wifi to sleep
            sleepyWiFi();
            
            // Adding a line to the bottom of the debug trace to denote the end of one loop
            if(logging >=1){Serial.println("===========================================================================");}
            break;
    }
}

void sleepyWiFi()
{
    // Putting the WiFi mondule into sleep mode, this saves battery and acts as a timer delay
    if(WiFi.status() == 2)
    {
        if(logging >1)
        {
            Serial.print("Putting the Wifi to sleep for ");
            if(secondsOfSleep < 120)
            {
                Serial.print(secondsOfSleep);
                Serial.println(" seconds.");
            } else {
                Serial.print(minutesOfSleep);
                Serial.println(" minutes.");
            }
        }
        
        Spark.sleep(secondsOfSleep);
    }
}

void wifiAsleep()
{
    if(logging >=2){Serial.print("Shhh, the WiFi is sleeping ");}
    
    // Print "Zs" while the WiFi is asleep
    while(WiFi.status() == 0)
    {
        if(logging >=2)
        {
            Serial.print("z");
            delay(1000);
        }
    }
}

void wifiConnecting()
{
    if(logging >=2)
    {
        Serial.println();
        Serial.print("Connecting to WiFi");
    }
    
    wifiTimeout = millis() + wifiReconnect;
    
    while(WiFi.status() == 1)
    {
        // While the WiFi is connecting display progress dots
        if(logging >= 1){Serial.print(".");}
        delay(500);
        
        // Handle if the WiFi fails to connected in time.
        if(millis() > wifiTimeout)
        {
            wifiTimeout = millis() + wifiReconnect;
            if(logging >=1)
            {
                Serial.println();
                Serial.println(" Failed to connect to wifi, restarting the Core.");
            }
            delay(500);
            // The Deep Sleep mode essentially reboots the core
            Spark.sleep(SLEEP_MODE_DEEP,5);
        }
    }
}

bool cloudConnected()
{
    if(!Spark.connected())
    {
        if(logging >=2)
        {
            Serial.print("Connecting to the Spark Cloud");
        }
        cloudTimeout = millis() + cloudReconnect;
    }
    
    // While trying to connect to the Spark Cloud
    while(!Spark.connected())
    {
        //While we are waiting for it to connect to the Spark Cloud display progrss dots
        if(logging >=2){Serial.print(".");}
        delay(500);
        
        // Handle if it failes to connect to the spark cloud in time
        if(millis() > cloudTimeout)
        {
            cloudTimeout = millis() + cloudReconnect;
            if(logging >=1)
            {
                Serial.println();
                Serial.println(" Failed to connect to the Spark Cloud, restarting the Core.");
            }
            // The Deep Sleep mode essentially reboots the core
            Spark.sleep(SLEEP_MODE_DEEP,5);
        }
    }
    
    // ==================================================
    //Once connected...
    if(Spark.connected())
    {
        return 1;
    }
}

/*------------------------------------------------
Sends sensor data to Thingspeak
Inputs: String, data to be entered for each field
Returns: 
------------------------------------------------*/
void ThingSpeakUpdate(String tsData)
{
    if(logging >= 1){Serial.println("...Attempting to update Thingspeak");}
    
    // Connecting and sending data to Thingspeak
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
        
        client.println(tsData); //the ""ln" is important here.
    
        // This delay is pivitol without it the TCP client will often close before the data is fully sent
        delay(100);
        
        if(logging >= 1)
        {
            Serial.print("Thingspeak update sent, ");
            Serial.println(nowDT());
        }
    }
    else{
        // Failed to connect to Thingspeak
        if(logging >= 1){Serial.println("Unable to connect to Thingspeak.");}
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
bool readDHT()
{
    // If no logging we need to get the DHT22 sensor at least 2 seconds to "warm up"
    if(logging <1){delay(3000);}
    
    if(logging >= 1){Serial.print("...Attempting to read the DHT22 sensor");}
    
    for(int x = 0; x < 3; x++)
    {
        // Request sensor data from the DHT22
        DHT22.acquire();
    	
        // Sets the timeout for attempting to aquire the DHT22 sensor reading
        dhtTimeout = millis() + dhtReconnect;
        
        while (DHT22.acquiring())
        {
            // While waiting for the DHT22 sensor to respond show progress dots
            if(logging >= 2)
            {
                Serial.print(".");
            }
            delay(10);
            
            // This sets a timeout incase the DHT22 doesn't respond we can exit
            if(millis() >= dhtTimeout)
            {
                break;
            }
    	}
    	if(logging >= 1){Serial.println();}
    	
    	int result = DHT22.getStatus();
    	
    	// Checks for valid return status from the sensor
        if (result == IDDHTLIB_OK)
        {
            if(logging >= 1){Serial.println("DHT22 read successfully.");}
            return 1;
        } else {
            if(x<3)
            {
                if(logging >= 1){Serial.println("Failed to read. Resetting DHT22's power.");}
                digitalWrite(dht22PowerPin, LOW);
                delay(500);
                digitalWrite(dht22PowerPin, HIGH);
                delay(2000);
            } else {
                return 0;
            }
    	}
    }
    return 0;
}

/*------------------------------------------------
Builds a Date/Time string for pretty debuging
Inputs: none
Returns: String of Date/Time
------------------------------------------------*/
String nowDT()
{
    String month = addZero(Time.month());
    String day = addZero(Time.day());
    String hour = addZero(Time.hour());
    String minute = addZero(Time.minute());
    String second = addZero(Time.second());
    
    String timeString = month+"/"+day+" "+hour+":"+minute+":"+second;
    
    return timeString;
}

/*------------------------------------------------
Adds a leading zero to a number less than 10
Inputs: integer
Returns: String
------------------------------------------------*/
String addZero(int data)
{
    String strData = String(data);
    
    if(data <10)
    {
        strData = "0"+strData;
    }
    return strData;
}
