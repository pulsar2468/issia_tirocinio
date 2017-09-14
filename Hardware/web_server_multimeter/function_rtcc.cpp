#include "rtcc.h"
#include "Arduino.h"
#include <Wire.h>


unsigned char data;

// Called to write bytes to RTC
void rtcc::WriteRTCByte(const unsigned char adr, const unsigned char data) {
  Wire.beginTransmission(0x6f);
  Wire.write(adr);
  Wire.write(data);
  Wire.endTransmission();
}


String rtcc::rtcTime() {
  data = ReadRTCByte(2); // get hours from rtc
  int hour = data & 0xff >> (2);
  data = ReadRTCByte(1); // get minutes from rtc
  int minute = data & 0xff >> (1);
  data = ReadRTCByte(0); // get seconds from rtc
  int second = data & 0xff >> (1);
  String rtcTime;
  if (hour < 10) {
    rtcTime += "0";
  }
  rtcTime += String(hour, HEX);
  rtcTime += ":";
  if (minute < 10) {
    rtcTime += "0";
  }
  rtcTime += String(minute, HEX);
  rtcTime += ":";
  if (second < 10) {
    rtcTime += "0";
  }
  rtcTime += String(second, HEX);
  return rtcTime;
}

// Return the date as a string in format dd:mm:yy
String rtcc::rtcDate() {
  data = ReadRTCByte(4); // get day from rtc
  int day = data & 0xff >> (2);
  data = ReadRTCByte(5); // get month from rtc
  int month = data & 0xff >> (3);
  data = ReadRTCByte(6); // get year from rtc
  int year = data & 0xff >> (0);
  //build date string
  String rtcDate;
  rtcDate += String(day, HEX);
  rtcDate += "-";
  rtcDate += String(month, HEX);
  rtcDate += "-";
  rtcDate += String(year, HEX);
  return rtcDate;
}

unsigned char rtcc::ReadRTCByte(const unsigned char adr) {
  unsigned char data;
  Wire.beginTransmission(0x6f);
  Wire.write(adr);
  Wire.endTransmission();
  Wire.requestFrom(0x6f, 1);
  while (Wire.available()) data = Wire.read();
  return data;
}



