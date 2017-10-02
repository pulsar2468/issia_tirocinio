//ws_support_fcns.h

#ifndef WS_SUPPORT_FCNS_H_
#define WS_SUPPORT_FCNS_H_

#include <Arduino.h>
#include <assert.h>

//operating mode
#define PROGRAM_EEPROM 0
#define DEBUG_FAKE_EEPROM 0
#define DEBUG_FAKE_BROKER 1
#define DEBUG_FAKE_MSG 1
#define VERBOSE 1
#define PARTIAL_EXEC_TIME 0
#define TLOOP_US 1000000
//at least 333333 for board_type=0xF0
//at least 500000 for board_type=0xFE
//wdt resets board after about 2 sec

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

//HW peripherals
#define EEPROM_SIZE 128
#define EEPROM_ADDR 0x57
#define RTCC_ADDR 0x6F
#define I2C_SENSOR_ADDR 0x6F
#define I2C_SENSOR_REG 0x05

//ADC parameters
#define NSAMPLES 1000
#define TSAMPLE_US 200
//sampling freq. is 5 kHz (max 8 kHz)
#define ADC_CALIB_GAIN 1.0625
//#define ADC_CALIB_GAIN 1


//type definitions:
//signal values, i.e., either 8 raw measured data
//or Vdc, Vrms, Idc, Irms, Pdc, A, P, T
#define CHANNELS_LEN 11
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

//msg id for messages sent by remote client
#define MSG_ID_WHO_ARE_YOU 0x68
/* msg format
struct {
  byte msg_id = MSG_ID_WHO_ARE_YOU
  byte current_board_id or 0xFF for broadcast;
  byte verbose;
}
*/
#define MSG_ID_CONFIG 0x63
/* msg format
struct {
  byte msg_id = MSG_ID_CONFIG
  byte current_board_id or 0xFF for broadcast;
  byte board_type;
  byte MQTT_IPaddr_3; 
  ...see below...
}
*/
#define MSG_ID_SETDATETIME 0x05
/* msg format
struct {
  byte msg_id = MSG_ID_SETDATETIME
  byte current_board_id or 0xFF for broadcast;
  byte yy;
  byte mth;
  byte dd;
  byte hh;
  byte mm;
  byte ss;
};
*/

//config data sent by remote client and stored in EEPROM
#define CONFIG_DATA_STR_START_IDX 8
#define CONFIG_DATA_STR_SIZE 30
#define CONFIG_DATA_STR_NUM 4
#define CONFIG_DATA_LEN (CONFIG_DATA_STR_START_IDX + CONFIG_DATA_STR_NUM * CONFIG_DATA_STR_SIZE)
#define RXDATA_BUFSIZE (CONFIG_DATA_LEN + 3)
//pay attention to byte alignment
/*
  byte msg_id = MSG_ID_CONFIG
  byte board_id = current_board_id or 0xFF for broadcast
  other bytes as following
*/
struct config_data_t {
  byte board_id;
  byte board_type;
  byte MQTT_IPaddr_3;
  byte MQTT_IPaddr_2;
  byte MQTT_IPaddr_1;
  byte MQTT_IPaddr_0;
  byte MQTT_port_1;
  byte MQTT_port_0;
  char MQTT_user[CONFIG_DATA_STR_SIZE];
  char MQTT_pwd[CONFIG_DATA_STR_SIZE];
  char WiFi_SSID[CONFIG_DATA_STR_SIZE];
  char WiFi_pwd[CONFIG_DATA_STR_SIZE];
};

//msg id for messages sent by wireless sensor
#define MSG_ID_NEWDATA 0x22
//measured data sent by wireless sensor to remote client
#define TXDATA_LEN 53
#define TXDATA_CH_START_IDX 9
//pay attention to byte alignment
struct txdata_t {
  byte msg_id;
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
bool program_eeprom(byte *aconfig_data);
void read_config_data_from_eeprom(byte *aconfig_data);
byte ReadI2CByte(byte addr, byte reg);
void WriteI2CByte(byte addr, byte reg, byte data);
void WriteSPIByte(byte value);
void toggle_error_led(int pin);
void toggle_confirmation_led(int pin);
void set_mux_ch(unsigned int ch);
void acquire_raw_analog_channels(struct channels_t *channels);
void acquire_and_process_v_and_i(struct channels_t *channels);
float compute_period(float *v, float *signs);

String rtcDate(void);
String rtcTime(void);
void timestamp(void);
void print_elapsed_time(String msg, unsigned long start_time_us, unsigned long stop_time_us);
void splitIPaddress(char *ipstr, byte *addr3, byte *addr2, byte *addr1, byte *addr0);
void buildIPaddress(char *ipstr, byte addr3, byte addr2, byte addr1, byte addr0);
void splitIPport(unsigned int port, byte *hi, byte *lo);
unsigned int buildIPport(byte hi, byte lo);

static_assert(CONFIG_DATA_LEN <= EEPROM_SIZE,
      "EEPROM size is too small. Can't compile.");
#endif /* WS_SUPPORT_FCNS_H_ */

