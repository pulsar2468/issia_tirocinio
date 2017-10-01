//ws_support_fcns.h
//http://www.hobbytronics.co.uk/arduino-external-eeprom

#ifndef WS_SUPPORT_FCNS_H_
#define WS_SUPPORT_FCNS_H_

#include <Arduino.h>

#define PROGRAM_EEPROM 0
#define DEBUG_FAKE_EEPROM 1
#define DEBUG_FAKE_BROKER 1
#define DEBUG_FAKE_MSG 1
#define VERBOSE 1
#define PRINT_EXEC_TIME 0
#define NSAMPLES 1000
#define TSAMPLE_US 200
//sampling freq. is 5 kHz (max 8 kHz)
#define TLOOP_US 1000000
//at least 333333 for board_type=0xF0
//at least 500000 for board_type=0xFE
//wdt resets board after about 2 sec
#define CALIB_GAIN 1.0564
//#define CALIB_GAIN 1
#define RTCC_ADDR 0x6F
#define EEPROM_ADDR 0x57
#define I2C_SENSOR_ADDR 0x6F
#define I2C_SENSOR_REG 0x05

//pin definitions
#define PIN_LED D4
#define PIN_MUX_S0 D0
#define PIN_MUX_S1 D3
#define PIN_MUX_S2 D8
#define PIN_1WIRE D4
#define PIN_I2C_SCL D1
#define PIN_I2C_SDA D2
#define PIN_SPI_SCLK D5
#define PIN_SPI_MISO D6
#define PIN_SPI_MOSI D7
#define PIN_SPI_SSn D8

//msg id for messages sent by remote client
#define MSG_ID_WHO_ARE_YOU 0x68
#define MSG_ID_CONFIG 0x63
#define MSG_ID_SETDATETIME 0x05
#define MSG_ID_QUERY 0x71

//msg id for messages sent by wireless sensor
#define MSG_NEWDATA 0x22

//type definitions:
//config data sent by client and stored in EEPROM
#define EEPROM_SIZE 512
#define EEPROM_VAR_START_IDX 8
#define EEPROM_STR_SIZE 125
#define LEN_CONFIG_DATA_EE 0
#define RXDATA_BUFSIZE (EEPROM_SIZE+8)
struct config_data_t {
  byte board_id;
  byte board_type;
  byte MQTT_IPaddr_3;
  byte MQTT_IPaddr_2;
  byte MQTT_IPaddr_1;
  byte MQTT_IPaddr_0;
  byte MQTT_port_1;
  byte MQTT_port_0;
  byte MQTT_user_len;
  char MQTT_user[EEPROM_STR_SIZE];
  byte MQTT_pwd_len;
  char MQTT_pwd[EEPROM_STR_SIZE];
  byte WiFi_SSID_len;
  char WiFi_SSID[EEPROM_STR_SIZE];
  byte WiFi_pwd_len;
  char WiFi_pwd[EEPROM_STR_SIZE];
};

//signal values, i.e., either 8 raw measured data
//or Vdc, Vrms, Idc, Irms, Pdc, P, A, T
#define LEN_CHANNELS 11
struct channels_t {
  float ch_a0;
  float ch_a1;
  float ch_a2;
  float ch_a3;
  float ch_a4;
  float ch_a5;
  float ch_a6;
  float ch_a7;
  float ch_1wire;
  float ch_i2c;
  float ch_spi;
};

//measured data sent by wireless sensor to remote client
#define LEN_TXDATA 53
#define CH_START_INDEX 9
//pay attention to 4-byte alignment
struct txdata_t {
  byte cmd;
  byte board_id;
  byte board_type;
  byte yy;
  byte mth;
  byte dd;
  byte hh;
  byte mm;
  byte ss;
  float ch_a0;
  float ch_a1;
  float ch_a2;
  float ch_a3;
  float ch_a4;
  float ch_a5;
  float ch_a6;
  float ch_a7;
  float ch_1wire;
  float ch_i2c;
  float ch_spi;
};

//message sent by remote client to set the date and time
struct datetime_t {
  byte msg_id;
  byte board_id;
  byte yy;
  byte mth;
  byte dd;
  byte hh;
  byte mm;
  byte ss;
};
  
//function prototypes
void read_config_data_from_eeprom(struct config_data_t *config_data);
bool program_eeprom(byte *aconfig_data);
void toggle_error_led(int pin);
void toggle_confirmation_led(int pin);
void set_mux_ch(unsigned int ch);
void acquire_raw_analog_channels(struct channels_t *channels);
void acquire_and_process_analog_channels(struct channels_t *channels);
float compute_period(float *v, float *signs);
unsigned char ReadI2CByte(const unsigned char addr, const unsigned char reg);
void WriteI2CByte(const unsigned char addr, const unsigned char reg, 
                  const unsigned char data);
String rtcDate(void);
String rtcTime(void);
void timestamp(void);
void print_elapsed_time(String msg, unsigned long start_time_us, unsigned long stop_time_us);
void spiWrite(byte value);

#endif /* WS_SUPPORT_FCNS_H_ */

