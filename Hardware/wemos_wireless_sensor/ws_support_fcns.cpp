//ws_support_fcns.c

#include "ws_support_fcns.h"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>


//function implementations
//
bool program_eeprom(byte *aconfig_data) {
  byte data;

  //program
  if (!DEBUG_FAKE_EEPROM) {
    for (int i = 0; i < CONFIG_DATA_LEN; i++) {
      WriteI2CByte(EEPROM_ADDR, 0x00 + i, aconfig_data[i]);
    }
  }
  else {
    Serial.println("Simulated EEPROM write:");
    char mybuffer[200];
    for (int offset = 0; offset < 16; offset++) {
      for (int i = 0; i < 32; i++) {
        snprintf(mybuffer, sizeof(mybuffer), "0x%02x, ", aconfig_data[offset * 32 + i]);
        Serial.print(mybuffer);
      }
      Serial.println();
    }
  }
  Serial.println("EEPROM programmed!");

  //verify
  if (!DEBUG_FAKE_EEPROM) {
    for (int i = 0; i < CONFIG_DATA_LEN; i++) {
      data = ReadI2CByte(EEPROM_ADDR, 0x00 + i);
      if (data != aconfig_data[i]) {
        Serial.println("EEPROM failed verify!");
        return false;
      }
    }
  }
  Serial.println("EEPROM verified!");
  return true;
}

//*****************************************************************************

void read_config_data_from_eeprom(byte *aconfig_data) {
  aconfig_data[0] = ReadI2CByte(EEPROM_ADDR, 0x00);
  aconfig_data[1] = ReadI2CByte(EEPROM_ADDR, 0x01);
  aconfig_data[2] = ReadI2CByte(EEPROM_ADDR, 0x02);
  aconfig_data[3] = ReadI2CByte(EEPROM_ADDR, 0x03);
  aconfig_data[4] = ReadI2CByte(EEPROM_ADDR, 0x04);
  aconfig_data[5] = ReadI2CByte(EEPROM_ADDR, 0x05);
  aconfig_data[6] = ReadI2CByte(EEPROM_ADDR, 0x06);
  aconfig_data[7] = ReadI2CByte(EEPROM_ADDR, 0x07);

  for (int j = 0; j < CONFIG_DATA_STR_NUM; j++) {
    for (int i = 0; i < CONFIG_DATA_STR_SIZE; i++) {
      aconfig_data[CONFIG_DATA_STR_START_IDX + j * CONFIG_DATA_STR_SIZE + i] =
        ReadI2CByte(EEPROM_ADDR, CONFIG_DATA_STR_START_IDX + j * CONFIG_DATA_STR_SIZE + i);
    }
  }
}

//*****************************************************************************
// Called to read bytes from I2C
byte ReadI2CByte(byte addr, byte reg) {
  byte data = 0xFF;

  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();

  //implement timeout of 5 ms
  unsigned long s_time = millis();
  while ((millis() - s_time) < 5) {
    Wire.requestFrom((int) addr, 1);
    if (Wire.available()) {
      data = Wire.read();
      break;
    }
  }
  return data;
}

//*****************************************************************************
// Called to write bytes to I2C
void WriteI2CByte(byte addr, byte reg, byte data) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
  delay(5); //bus free time
}

//*****************************************************************************

void WriteSPIByte(byte value) {
  // take the SS pin low to select the chip:
  digitalWrite(PIN_SPI_SSn, LOW);
  // send in the value via SPI:
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(PIN_SPI_SSn, HIGH);
  delay(5);
}

//*****************************************************************************

void toggle_error_led(int pin) {
  //flash LED at 2 Hz and do not return
  pinMode(PIN_LED, OUTPUT);
  while (1) {
    digitalWrite(pin, HIGH);
    delay(250);
    digitalWrite(pin, LOW);
    delay(250);
  }
  pinMode(PIN_1WIRE, INPUT_PULLUP);
}

//*****************************************************************************

void toggle_confirmation_led(int pin) {
  //flash LED at 0.5 Hz and do not return
  pinMode(PIN_LED, OUTPUT);
  while (1) {
    digitalWrite(pin, HIGH);
    delay(1000);
    digitalWrite(pin, LOW);
    delay(1000);
  }
  pinMode(PIN_1WIRE, INPUT_PULLUP);
}

//*****************************************************************************

void set_mux_ch(unsigned int ch) {
  switch (ch) {
    case 0:
      digitalWrite(PIN_MUX_S0, LOW);
      digitalWrite(PIN_MUX_S1, LOW);
      digitalWrite(PIN_MUX_S2, LOW);
      break;
    case 1:
      digitalWrite(PIN_MUX_S0, HIGH);
      digitalWrite(PIN_MUX_S1, LOW);
      digitalWrite(PIN_MUX_S2, LOW);
      break;
    case 2:
      digitalWrite(PIN_MUX_S0, LOW);
      digitalWrite(PIN_MUX_S1, HIGH);
      digitalWrite(PIN_MUX_S2, LOW);
      break;
    case 3:
      digitalWrite(PIN_MUX_S0, HIGH);
      digitalWrite(PIN_MUX_S1, HIGH);
      digitalWrite(PIN_MUX_S2, LOW);
      break;
    case 4:
      digitalWrite(PIN_MUX_S0, LOW);
      digitalWrite(PIN_MUX_S1, LOW);
      digitalWrite(PIN_MUX_S2, HIGH);
      break;
    case 5:
      digitalWrite(PIN_MUX_S0, HIGH);
      digitalWrite(PIN_MUX_S1, LOW);
      digitalWrite(PIN_MUX_S2, HIGH);
      break;
    case 6:
      digitalWrite(PIN_MUX_S0, LOW);
      digitalWrite(PIN_MUX_S1, HIGH);
      digitalWrite(PIN_MUX_S2, HIGH);
      break;
    case 7:
      digitalWrite(PIN_MUX_S0, HIGH);
      digitalWrite(PIN_MUX_S1, HIGH);
      digitalWrite(PIN_MUX_S2, HIGH);
      break;
    default:
      digitalWrite(PIN_MUX_S0, LOW);
      digitalWrite(PIN_MUX_S1, LOW);
      digitalWrite(PIN_MUX_S2, LOW);
      break;
  }
}

//*****************************************************************************

void acquire_raw_analog_channels(struct channels_t *channels) {
  set_mux_ch(0);
  delayMicroseconds(1);
  channels->ch_a0 = (float) analogRead(A0) * 3.2 * ADC_CALIB_GAIN / 1024;

  set_mux_ch(1);
  delayMicroseconds(1);
  channels->ch_a1 = (float) analogRead(A0) * 3.2 * ADC_CALIB_GAIN / 1024;

  set_mux_ch(2);
  delayMicroseconds(1);
  channels->ch_a2 = (float) analogRead(A0) * 3.2 * ADC_CALIB_GAIN / 1024;

  set_mux_ch(3);
  delayMicroseconds(1);
  channels->ch_a3 = (float) analogRead(A0) * 3.2 * ADC_CALIB_GAIN / 1024;

  set_mux_ch(4);
  delayMicroseconds(1);
  channels->ch_a4 = (float) analogRead(A0) * 3.2 * ADC_CALIB_GAIN / 1024;

  set_mux_ch(5);
  delayMicroseconds(1);
  channels->ch_a5 = (float) analogRead(A0) * 3.2 * ADC_CALIB_GAIN / 1024;

  set_mux_ch(6);
  delayMicroseconds(1);
  channels->ch_a6 = (float) analogRead(A0) * 3.2 * ADC_CALIB_GAIN / 1024;

  set_mux_ch(7);
  delayMicroseconds(1);
  channels->ch_a7 = (float) analogRead(A0) * 3.2 * ADC_CALIB_GAIN / 1024;
}

//*****************************************************************************

void acquire_and_process_v_and_i(struct channels_t *channels) {
  static int buf0[NSAMPLES];
  static int buf1[NSAMPLES];
  static float v[NSAMPLES];
  static float i[NSAMPLES];
  static float p[NSAMPLES];
  float sum_v = 0.0;
  float sum_i = 0.0;
  float sum_p = 0.0;
  float sum_squared_v = 0.0;
  float sum_squared_i = 0.0;
  float Vmean, Vrms, Imean, Irms, A, P, T, f;

  //store NSAMPLES samples of V and I in buf0 and buf1
  for (int j = 0; j < NSAMPLES; j++) {
    set_mux_ch(0);
    delayMicroseconds(1);
    buf0[j] = analogRead(A0);
    set_mux_ch(1);
    delayMicroseconds(1);
    buf1[j] = analogRead(A0);
  }

  //scale values and compute power and sums
  for (int j = 0; j < NSAMPLES; j++) {
    v[j] = (float) buf0[j] * (3.2 * ADC_CALIB_GAIN / 1024);
    i[j] = (float) buf1[j] * (3.2 * ADC_CALIB_GAIN / 1024);
    p[j] = v[j] * i[j];
    sum_v += v[j];
    sum_i += i[j];
    sum_p += p[j];
  }

  //compute Vmean, Imean, P
  Vmean = sum_v / NSAMPLES;
  Imean = sum_i / NSAMPLES;
  P = sum_p / NSAMPLES;

  //remove mean value and compute sums of squared elements
  for (int j = 0; j < NSAMPLES; j++) {
    v[j] -= Vmean;
    i[j] -= Imean;
    sum_squared_v += pow(v[j], 2);
    sum_squared_i += pow(i[j], 2);
  }

  //compute Vrms, Irms, A
  Vrms = sqrt(sum_squared_v / NSAMPLES);
  Irms = sqrt(sum_squared_i / NSAMPLES);
  A = Vrms * Irms;

  //reuse p to store sign of v (without mean value) and compute T and f
  T = compute_period(v, p);
  if (T == INFINITY) {
    f = 0.0;
  }
  else if (T == 0.0) {
    f = INFINITY;
  }
  else {
    f = 1.0 / T;
  }

  //return values
  channels->ch_a0 = Vmean;
  channels->ch_a1 = Vrms;
  channels->ch_a2 = Imean;
  channels->ch_a3 = Irms;
  channels->ch_a4 = A;
  channels->ch_a5 = P;
  channels->ch_a6 = T;
  channels->ch_a7 = f;

  if (1) {
    /*
      Serial.print("Vmean: ");
      Serial.println(Vmean, 3);
      Serial.print("Vrms: ");
      Serial.println(Vrms, 3);
      Serial.print("Imean: ");
      Serial.println(Imean, 3);
      Serial.print("Irms: ");
      Serial.println(Irms, 3);
      Serial.print("A: ");
      Serial.println(A, 2);
      Serial.print("P: ");
      Serial.println(P, 2);
      Serial.print("T: ");
      Serial.println(T, 5);
      Serial.print("f: ");
      Serial.println(f, 5);
      delayMicroseconds(10);
    */
  }
}

//*****************************************************************************

float compute_period(float *v, float *signs) {
  int flag = 0;
  float id[2] = {0, 0};

  for (int i = 0; i < NSAMPLES; i++) {
    //signs[i] = not(v[i] > 0.1 or v[i] < -0.1);
    signs[i] = (v[i] >= 0.0 ? 1.0 : 0.0);
  }

  for (int i = 0; i < NSAMPLES - 1; i++) {
    if ((signs[i] == 0) and (signs[i + 1] == 1)) {
      //interpolate index
      id[flag] =  v[i + 1] / (v[i] - v[i + 1]) + i + 1;
      flag++;
      if (flag == 1) {
        id[1] = id[0];
      }
      else if (flag == 2) {
        return (id[1] - id[0]) * TSAMPLE_US * 1e-6;
      }
    }
  }
  return INFINITY;
}

//*****************************************************************************
// Return the date as a string in format dd:mm:yyyy
String rtcDate(void) {
  unsigned char data;

  data = ReadI2CByte(RTCC_ADDR, 4); // get day from rtc
  byte day = data & 0xff >> (2);
  data = ReadI2CByte(RTCC_ADDR, 5); // get month from rtc
  byte month = data & 0xff >> (3);
  data = ReadI2CByte(RTCC_ADDR, 6); // get year from rtc
  byte year = data & 0xff >> (0);

  String rtcDate;
  if (day < 10) rtcDate += "0";
  rtcDate += String(day, HEX);
  rtcDate += "/";
  if (month < 10) rtcDate += "0";
  rtcDate += String(month, HEX);
  rtcDate += "/";
  rtcDate += "20";
  if (year < 10) rtcDate += "0";
  rtcDate += String(year, HEX);
  return rtcDate;
}

//*****************************************************************************
// Return the time as a string in format hh:mm:ss
String rtcTime(void) {
  unsigned char data;

  data = ReadI2CByte(RTCC_ADDR, 2); // get hours from rtc
  byte hour = data & 0xff >> (2);
  data = ReadI2CByte(RTCC_ADDR, 1); // get minutes from rtc
  byte minute = data & 0xff >> (1);
  data = ReadI2CByte(RTCC_ADDR, 0); // get seconds from rtc
  byte second = data & 0xff >> (1);

  String rtcTime;
  if (hour < 10) rtcTime += "0";
  rtcTime += String(hour, HEX);
  rtcTime += ":";
  if (minute < 10) rtcTime += "0";
  rtcTime += String(minute, HEX);
  rtcTime += ":";
  if (second < 10) rtcTime += "0";
  rtcTime += String(second, HEX);
  return rtcTime;
}

//*****************************************************************************

void timestamp(void) {
  Serial.print(rtcDate());
  Serial.print("  -  ");
  Serial.println(rtcTime());
  delayMicroseconds(10);
}

//*****************************************************************************

void print_elapsed_time(String msg, unsigned long start_time_us, unsigned long stop_time_us) {
  Serial.print(msg);
  Serial.print(stop_time_us - start_time_us);
  Serial.println(" us");
  delayMicroseconds(10);
}

//*****************************************************************************

void splitIPaddress(char *ipstr, byte *addr3, byte *addr2, byte *addr1, byte *addr0) {
  //TODO: split using regexpr
  *addr3 = 150;
  *addr2 = 145;
  *addr1 = 127;
  *addr0 = 37;
}

//*****************************************************************************

void buildIPaddress(char *ipstr, byte addr3, byte addr2, byte addr1, byte addr0) {
  sprintf(ipstr, "%u.%u.%u.%u", addr3, addr2, addr1, addr0);
}

//*****************************************************************************
void splitIPport(unsigned int port, byte *hi, byte *lo) {
  *hi = (port & 0x0000FF00) >> 8;
  *lo = (port & 0x000000FF);
}

//*****************************************************************************

unsigned int buildIPport(byte hi, byte lo) {
  return (((unsigned int) hi << 8) + lo);
}

