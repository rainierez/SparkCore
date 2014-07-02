/*--------------------------------------------------------
wifiSleep_Timer

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
An example sketch that uses the WiFi sleep as a basic timer, 
there are also built in timeouts for connecting to WiFi and 
the Spark cloud.

This is an easy way to save battery when uploading sensor 
data to the cloud.

--------------------------------------------------------*/

// Version numbering
#define VERSION 1.01

// Time that the WiFi will go into sleep (in second)
float minutesOfSleep = 5;
int secondsOfSleep = minutesOfSleep*60; //Convert to milliseconds

//reconnect timelimits (in milliseconds)
int wifiReconnect = 20000;
int cloudReconnect = 20000;
int wifiTimeout;
int cloudTimeout;

/*--------------------------------------------------------
Setup
--------------------------------------------------------*/
void setup()
{
    // Startup information logging
    Serial.begin(9600);
    delay(10000);
    
    Serial.println("===========================================================================");
    Serial.println("=== Starting ===");
    Serial.print("Version: ");
    Serial.println(VERSION);
    
    Serial.print("-Network SSID: ");
    Serial.println(Network.SSID());
    
    Serial.print("-Local IP: ");
    Serial.println(Network.localIP());
    
    if(Spark.connected())
    {
        Serial.println("Connected to Spark Cloud.");
    }
    Serial.println("===========================================================================");
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
            Serial.println("WiFi Connected.");
            if(cloudConnected())
            {
                Serial.println("...Uploading Data");
                /*--------------------------------------------------------
                Here you enter the code to upload the stored data to the cloud
                --------------------------------------------------------*/
                Serial.println("Data uploaded.");
            }
            
            // Put the Wifi to sleep
            sleepyWiFi();
            
            // Adding a line to the bottom of the debug trace to denote the end of one loop
            Serial.println("===========================================================================");
            break;
    }
}

void sleepyWiFi()
{
    // Putting the WiFi mondule into sleep mode, this saves battery and acts as a timer delay
    if(WiFi.status() == 2)
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
        
        Spark.sleep(secondsOfSleep);
    }
}

void wifiAsleep()
{
    Serial.print("Shhh, the WiFi is sleeping ");
    
    // Print "Zs" while the WiFi is asleep
    while(WiFi.status() == 0)
    {
        Serial.print("z");
        delay(1000);
        /*--------------------------------------------------------
        Here you enter the code that continues while the Wifi is
        asleep. Just take out the Serial.print and the delay above.
        --------------------------------------------------------*/
    }
}

void wifiConnecting()
{
    Serial.println();
    Serial.print("Connecting to WiFi");
    
    wifiTimeout = millis() + wifiReconnect;
    
    while(WiFi.status() == 1)
    {
        // While the WiFi is connecting display progress dots
        Serial.print(".");
        delay(500);
        
        // Handle if the WiFi fails to connected in time.
        if(millis() > wifiTimeout)
        {
            wifiTimeout = millis() + wifiReconnect;
            Serial.println();
            Serial.println(" Failed to connect to wifi, restarting the Core.");
            
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
        Serial.print("Connecting to the Spark Cloud");
        
        cloudTimeout = millis() + cloudReconnect;
    }
    
    // While trying to connect to the Spark Cloud
    while(!Spark.connected())
    {
        //While we are waiting for it to connect to the Spark Cloud display progrss dots
        Serial.print(".");
        delay(500);
        
        // Handle if it failes to connect to the spark cloud in time
        if(millis() > cloudTimeout)
        {
            cloudTimeout = millis() + cloudReconnect;
            Serial.println();
            Serial.println(" Failed to connect to the Spark Cloud, restarting the Core.");
            
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
