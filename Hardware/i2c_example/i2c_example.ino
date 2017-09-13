#include <Wire.h>
unsigned char data;

void setup()
{
   Serial.begin(9600);
   Wire.begin();
   
//Set the RTC   
  WriteRTCByte(0,0);       //STOP RTC
  WriteRTCByte(1,0);    //minute 0
  WriteRTCByte(2,0x17);    //HOUR
  WriteRTCByte(3,0x10);    //DAY=1(MONDAY) AND VBAT=1
  WriteRTCByte(4,0x10);    //DATE
  WriteRTCByte(5,0x12);    //MONTH
  WriteRTCByte(6,0x17);    //YEAR
  WriteRTCByte(0,0x80);    //START RTC, SECOND=00
  delay(100);
}

void loop(){
 //uint8 sec = ((x & B01110000) >> 4) * 10 + ((x & B00001111));
  Serial.println(rtcTime());
  Serial.println(rtcDate());
  delay(1000);
}

// Called to read bytes from RTC
  unsigned char ReadRTCByte(const unsigned char adr){
  unsigned char data;
  Wire.beginTransmission(0x6f);
  Wire.write(adr);
  Wire.endTransmission();
  Wire.requestFrom(0x6f,1);
  while (Wire.available()) data=Wire.read();
  return data;
}

// Called to write bytes to RTC
void WriteRTCByte(const unsigned char adr, const unsigned char data){
  Wire.beginTransmission(0x6f);
  Wire.write(adr);
  Wire.write(data);
  Wire.endTransmission();
} 


String rtcTime() { 
  data=ReadRTCByte(2); // get hours from rtc
  int hour=data & 0xff>>(2);
  data=ReadRTCByte(1); // get minutes from rtc
  int minute=data & 0xff>>(1);
  data=ReadRTCByte(0); // get seconds from rtc
  int second=data & 0xff>>(1);
  String rtcTime;
  if (hour < 10){rtcTime += "0";} 
  rtcTime += String(hour,HEX);
  rtcTime += ":";  
  if (minute < 10){rtcTime += "0";} 
  rtcTime += String(minute,HEX);  
  rtcTime += ":";  
  if (second < 10){rtcTime += "0";} 
  rtcTime += String(second,HEX);
  return rtcTime;  
}

// Return the date as a string in format dd:mm:yy
String rtcDate() { 
  data=ReadRTCByte(4); // get day from rtc
  int day=data & 0xff>>(2);
  data=ReadRTCByte(5); // get month from rtc
  int month=data & 0xff>>(3);
  data=ReadRTCByte(6); // get year from rtc
  int year=data & 0xff>>(0);
  //build date string
  String rtcDate;
  rtcDate += String(day,HEX);
  rtcDate += ":";  
  rtcDate += String(month,HEX);  
  rtcDate += ":";  
  rtcDate += String(year,HEX);
  return rtcDate;  
}
