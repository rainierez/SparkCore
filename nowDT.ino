void setup() {
    Serial.begin(9600);
    delay(1000);
    Serial.println("===Starting===");
}

void loop() {
    Serial.println(nowDT());
    delay(1000);
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
