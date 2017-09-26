//ws_support_fcns.h

#ifndef WS_SUPPORT_FCNS_H_
#define WS_SUPPORT_FCNS_H_

#include <Arduino.h>

#define PROGRAM_EEPROM 0
#define DEBUG_WITHOUT_EEPROM 1
#define MSG_ARRIVED 0
#define VERBOSE 1
#define NSAMPLES 1000
#define TSAMPLE_US 200
//sampling freq. is 5 kHz (max 8 kHz)
#define TLOOP_US 1000000
//wdt resets board after about 2 sec
#define CALIB_GAIN 1.0538
#define RTCC_ADDR 0x6F
#define EEPROM_ADDR 0x57
#define I2C_SENSOR_ADDR 0x6F
#define I2C_SENSOR_REG 0x05

//pin definitions
#define PIN_MUX_S0 D0
#define PIN_MUX_S1 D3
#define PIN_MUX_S2_SPI_SSn D8
#define PIN_1WIRE D4
#define PIN_LED D4
#define PIN_I2C_SCL D1
#define PIN_I2C_SDA D2
#define PIN_SPI_SCLK D5
#define PIN_SPI_MISO D6
#define PIN_SPI_MOSI D7

//msg id for messages sent by server
#define MSG_ID_QUERY_SENSORS 0x01
#define MSG_ID_CONFIG 0xF5

//msg id for messages sent by wireless sensor
#define MSG_ID_DATA 0x02

//type definitions:
//config data sent by server and stored in EEPROM
#define LEN_CONFIG_DATA 15
#define LEN_CONFIG_DATA_EE 9
struct config_data_t {
  byte cmd;
  byte board_id;
  byte board_type;
  byte IPaddr_3;
  byte IPaddr_2;
  byte IPaddr_1;
  byte IPaddr_0;
  byte IPport_1;
  byte IPport_0;
  byte yy;
  byte mth;
  byte dd;
  byte hh;
  byte mm;
  byte ss;
};

//signal values, i.e., either raw measured data
//or Vmean, Vrms, Imean, Irms, A, P, T, f
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

//measured data sent by wireless sensor to server
#define LEN_TXDATA 53
#define CH_START_INDEX 9
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


//function prototypes
void read_config_data_from_eeprom(struct config_data_t *config_data);
bool program_eeprom(struct config_data_t *config_data);
void toggle_confirmation_led(int pin);
void toggle_error_led(int pin);
void set_mux_ch(unsigned int ch);
void acquire_raw_analog_channels(struct channels_t *channels);
void acquire_and_process_analog_channels(struct channels_t *channels);
float compute_period(float *v, float *signs);
unsigned char ReadI2CByte(const unsigned char addr, const unsigned char reg);
void WriteI2CByte(const unsigned char addr, const unsigned char reg, 
                  const unsigned char data);
String rtcDate(void);
String rtcTime(void);
void timeStamp(void);
void print_elapsed_time(String msg, unsigned long start_time_us, unsigned long stop_time_us);
void spiWrite(byte value);

#endif /* WS_SUPPORT_FCNS_H_ */
