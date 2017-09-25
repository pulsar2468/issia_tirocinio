//wemos_wireless_sensor.ino

#include "ws_support_fcns.h"
#include <Wire.h>
#include "DHT.h"
#define DHTTYPE DHT22
DHT dht(PIN_1WIRE, DHTTYPE);

void setup()
{
  //setup serial port
  Serial.begin(115200);
  //setup GPIO pins
  pinMode(PIN_MUX_S0, OUTPUT);
  pinMode(PIN_MUX_S1_LED, OUTPUT);
  pinMode(PIN_MUX_S2_SPI_SSn, OUTPUT);
  //setup 1-Wire
  dht.begin();
  //setup I2C
  Wire.begin();
  //setup SPI master
  pinMode(PIN_SPI_SCLK, OUTPUT);
  pinMode(PIN_SPI_MISO, INPUT);
  pinMode(PIN_SPI_MOSI, OUTPUT);
  pinMode(PIN_MUX_S2_SPI_SSn, OUTPUT);
}

void loop()
{
  static bool first_time = true;
  static unsigned long initial_time_us, start_time_us, stop_time_us;
  static byte msg[] = {0x5F, 0x99, 0xAB}; //to be changed
  
  static struct config_data_t config_data = {0xF5, 33, 'O', 192, 168, 0, 200,
                                          0x17, 0x09, 0x25, 0x10, 0x30, 0x15};
  //static byte *aconfig_data = NULL; // aconfig_data is an alias of config_data
  static struct channels_t channels;
  static float *achannels = NULL; // achannels is an alias of channels
  static struct txdata_t txdata;
  static byte *atxdata = NULL; //atxdata is an alias of txdata
  
  //aconfig_data = (byte *) &config_data; // alias of config_data as an array
  achannels = (float *) &channels; // alias of channels as an array
  atxdata = (byte *) &atxdata; // alias of txdata as an array

  //PROGRAMMING MODE
  if (PROGRAM_EEPROM == 1) {
    //use default config_data
    if (program_eeprom(&config_data)) {
      toggle_confirmation_led(PIN_MUX_S1_LED); // do not return
    }
    else {
      toggle_error_led(PIN_MUX_S1_LED); // do not return   
    }
    return;
  }

  //NORMAL OPERATION MODE - FIRST CALL
  if (first_time) {
    first_time = false;
    if (DEBUG_WITHOUT_EEPROM == 0) {
      read_config_data_from_eeprom(&config_data);
      if (config_data.cmd != MSG_ID_CONFIG) {
        Serial.println("empty EEPROM");
        delayMicroseconds(10);
        toggle_error_led(PIN_MUX_S1_LED); // do not return
      }
    }
    // else use config initialization values if DEBUG_WITHOUT_EEPROM = 1
      
    // send welcome msg to broker, containing config_data
    // subscribe to config msgs sent by broker
    return;
  }

  //NORMAL OPERATION MODE - SUBSEQUENT CALLS
  initial_time_us = micros();
  Serial.println("starting loop...");
  delayMicroseconds(10);
  
  //STEP 1: check if new msg received
  //get message
  if (1) {
    start_time_us = micros();
    Serial.print("msg received: ");
    Serial.println(msg[0]);
    delayMicroseconds(10);
    
    if (msg[0] == MSG_ID_QUERY_SENSORS) {
      //send welcome msg to broker, containing config_data
    }
    else if (msg[0] == MSG_ID_CONFIG) {
      //program RTCC
      WriteI2CByte(RTCC_ADDR, 0x00, 0x00);    //stop RTCC and reset seconds
      WriteI2CByte(RTCC_ADDR, 0x06, config_data.yy);
      WriteI2CByte(RTCC_ADDR, 0x05, config_data.mth);
      WriteI2CByte(RTCC_ADDR, 0x04, config_data.dd);
      WriteI2CByte(RTCC_ADDR, 0x02, config_data.hh);
      WriteI2CByte(RTCC_ADDR, 0x01, config_data.mm);
      WriteI2CByte(RTCC_ADDR, 0x00, config_data.ss || 0x80);  //write seconds and start RTCC
      
      //store config_data in EEPROM
      if (!program_eeprom(&config_data)) {
        toggle_error_led(PIN_MUX_S1_LED); // do not return
      }
    }
    stop_time_us = micros();
    print_elapsed_time("msg processing finished in ", start_time_us, stop_time_us);
  }
  
  //STEP 2: acquire data from hw sensors
  if (config_data.board_type == 'E') {
    start_time_us = micros();
    acquire_and_process_analog_channels(&channels);
    stop_time_us = micros();
    print_elapsed_time("electrical acquisition finished in ", start_time_us, stop_time_us);
  }
  else {
    //acquire raw analog signals
    start_time_us = micros();
    acquire_raw_analog_channels(&channels);
    stop_time_us = micros();
    print_elapsed_time("analog ch acquisition finished in ", start_time_us, stop_time_us);
    
    //acquire 1-Wire data
    start_time_us = micros();
    channels.ch_1wire = dht.readTemperature();
    stop_time_us = micros();
    print_elapsed_time("1-Wire acquisition finished in ", start_time_us, stop_time_us);
    
    //acquire I2C data
    start_time_us = micros();
    channels.ch_i2c = ReadI2CByte(I2C_SENSOR_ADDR, I2C_SENSOR_REG);
    stop_time_us = micros();
    print_elapsed_time("I2C acquisition finished in ", start_time_us, stop_time_us);
    
    //acquire SPI data
    start_time_us = micros();
    channels.ch_spi = 0.0;  // to be defined
    stop_time_us = micros();
    print_elapsed_time("SPI acquisition finished in ", start_time_us, stop_time_us);
  }

  //STEP 3: retrieve timestamp and prepare data
  byte data;

  start_time_us = micros();
  txdata.cmd = MSG_ID_DATA;
  txdata.board_id = config_data.board_id;
  txdata.board_type = config_data.board_type;
  
  data=ReadI2CByte(RTCC_ADDR, 6); // get year from rtc
  txdata.yy = data & 0xff>>(0);
  data=ReadI2CByte(RTCC_ADDR, 5); // get month from rtc
  txdata.mth = data & 0xff>>(3);
  data=ReadI2CByte(RTCC_ADDR, 4); // get day from rtc
  txdata.dd = data & 0xff>>(2);

  data=ReadI2CByte(RTCC_ADDR, 2); // get hours from rtc
  txdata.hh = data & 0xff>>(2);    
  data=ReadI2CByte(RTCC_ADDR, 1); // get minutes from rtc
  txdata.mm = data & 0xff>>(1);
  data=ReadI2CByte(RTCC_ADDR, 0); // get seconds from rtc
  txdata.ss = data & 0xff>>(1);

  for (int i=0; i<LEN_CHANNELS; i++) {
    //store the 4 bytes of the float number
    atxdata[CH_START_INDEX + 2*i + 0] = (byte) achannels[i]; //to be fixed
    atxdata[CH_START_INDEX + 2*i + 1] = (byte) achannels[i];
    atxdata[CH_START_INDEX + 2*i + 2] = (byte) achannels[i];
    atxdata[CH_START_INDEX + 2*i + 3] = (byte) achannels[i];
  }
  stop_time_us = micros();
  print_elapsed_time("data preparing finished in ", start_time_us, stop_time_us);

  //STEP 4: send TxData to broker
  start_time_us = micros();
  //... LEN_TXDATA bytes
  for (int i=0; i<LEN_TXDATA-1; i++) {
    Serial.print(atxdata[i]);
    Serial.print(" - ");
  }
  Serial.println(atxdata[LEN_TXDATA-1]);
  delayMicroseconds(10);
  stop_time_us = micros();
  print_elapsed_time("sending msg finished in ", start_time_us, stop_time_us);

  //print total time
  stop_time_us = micros();
  print_elapsed_time("loop iteration finished in ", initial_time_us, stop_time_us);
  Serial.println(" ");
  delayMicroseconds(10);  
}

