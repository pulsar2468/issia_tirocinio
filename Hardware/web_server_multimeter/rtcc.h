#ifndef rtcc_h
#define rtcc_h
#include "Arduino.h"

//define class rtcc
class rtcc {
  public:
    //f_list(); // Costructor
    String rtcTime();
    String rtcDate();
    void WriteRTCByte(const unsigned char, const unsigned char);
    unsigned char ReadRTCByte(const unsigned char);
};

#endif
