//wemos_wireless_sensor.ino

#include "ws_support_fcns.h"
#include <SPI.h>
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
  pinMode(PIN_MUX_S1, OUTPUT);
  pinMode(PIN_MUX_S2_SPI_SSn, OUTPUT);
  //setup 1-Wire
  dht.begin();
  //setup I2C
  Wire.setClock(400000);
  Wire.begin();
  //setup SPI master
//  pinMode(PIN_SPI_SCLK, OUTPUT);
//  pinMode(PIN_SPI_MISO, INPUT);
//  pinMode(PIN_SPI_MOSI, OUTPUT);
  pinMode(PIN_MUX_S2_SPI_SSn, OUTPUT);
  SPI.begin();
}

void loop()
{
  static bool toggle_flag = false;
  static byte msg[] = {0xF5, 0x99, 0xAB}; //to be changed
  static bool first_time = true;
  static char mybuffer[55]; //a buffer to build formatted strings
  static unsigned long initial_time_us, start_time_us, stop_time_us;

  static struct config_data_t config_data = {0xF5, 33, 'E', 150, 145, 127, 37,
    0x22, 0xB3, 0x17, 0x09, 0x26, 0x10, 0x30, 0x15};
  //static byte *aconfig_data = NULL; // aconfig_data is an alias of config_data
  static struct channels_t channels;
  static float *achannels = NULL; // achannels is an alias of channels
  static struct txdata_t txdata;
  static byte *atxdata = NULL; //atxdata is an alias of txdata

  //aconfig_data = (byte *) &config_data; // alias of config_data as an array
  achannels = (float *) &channels; // alias of channels as an array
  atxdata = (byte *) &txdata; // alias of txdata as an array

  //PROGRAMMING MODE
  if (PROGRAM_EEPROM == 1) {
    //program default config_data in eeprom
    if (program_eeprom(&config_data)) {
      toggle_confirmation_led(PIN_LED); // do not return
    }
    else {
      toggle_error_led(PIN_LED); // do not return
    }
  }

  //NORMAL OPERATION MODE - FIRST CALL
  if (first_time) {
    first_time = false;
    if (DEBUG_WITHOUT_EEPROM == 0) {
      read_config_data_from_eeprom(&config_data);
      if (config_data.cmd != MSG_ID_CONFIG) {
        Serial.println("empty EEPROM");
        delayMicroseconds(10);
        toggle_error_led(PIN_LED); // do not return
      }
    }
    // else use config initialization values if DEBUG_WITHOUT_EEPROM = 1

    // send welcome msg to broker, containing config_data
    // subscribe to config msgs sent by broker
    if (VERBOSE) {
      Serial.println("exiting from first-time loop\n");
      delayMicroseconds(10);
    }
    return;
  }

  //NORMAL OPERATION MODE - SUBSEQUENT CALLS
  initial_time_us = micros();
  if (VERBOSE) {
    Serial.println("starting loop...");
    delayMicroseconds(10);
  }
  
  //STEP 1: check if new msg received
  //get message
  if (MSG_ARRIVED) {
    start_time_us = micros();
    Serial.print("msg received: ");
    snprintf(mybuffer, sizeof(mybuffer), "0x%02x", msg[0]);
    Serial.println(mybuffer);
    delayMicroseconds(10);

    if (msg[0] == MSG_ID_QUERY_SENSORS) {
      //send welcome msg to broker, containing config_data
      Serial.println("welcome!");
      delayMicroseconds(10);
    }
    else if (msg[0] == MSG_ID_CONFIG) {
      //program RTCC
      WriteI2CByte(RTCC_ADDR, 0x00, 0x00);    //stop RTCC and reset seconds
      WriteI2CByte(RTCC_ADDR, 0x06, config_data.yy);
      WriteI2CByte(RTCC_ADDR, 0x05, config_data.mth);
      WriteI2CByte(RTCC_ADDR, 0x04, config_data.dd);
      WriteI2CByte(RTCC_ADDR, 0x02, config_data.hh);
      WriteI2CByte(RTCC_ADDR, 0x01, config_data.mm);
      WriteI2CByte(RTCC_ADDR, 0x00, config_data.ss | 0x80);  //write seconds and start RTCC
      Serial.println("RTCC programmed");
      delayMicroseconds(10);
      
      //store config_data in EEPROM
      if (!program_eeprom(&config_data)) {
        toggle_error_led(PIN_LED); // do not return
      }
    }
    stop_time_us = micros();
    print_elapsed_time("msg processing finished in ", start_time_us, stop_time_us);
  }

  //STEP 2: acquire data from hw sensors
  //acquire analog channels
  if (config_data.board_type == 'E') {
    //acquire ch_0 and ch_1 and compute other quantities
    start_time_us = micros();
    acquire_and_process_analog_channels(&channels);
    stop_time_us = micros();
    if (VERBOSE)
      print_elapsed_time("electrical acquisition finished in ", start_time_us, stop_time_us);
  }
  else {
    //acquire raw analog signals
    start_time_us = micros();
    acquire_raw_analog_channels(&channels);
    stop_time_us = micros();
    if (VERBOSE) 
      print_elapsed_time("analog ch acquisition finished in ", start_time_us, stop_time_us);
  }
  //acquire 1-Wire data
  start_time_us = micros();
  channels.ch_1wire = dht.readTemperature(); 
  stop_time_us = micros();
  if (VERBOSE)
    print_elapsed_time("1-Wire acquisition finished in ", start_time_us, stop_time_us);

  //acquire I2C data
  start_time_us = micros();
  channels.ch_i2c = ReadI2CByte(I2C_SENSOR_ADDR, I2C_SENSOR_REG);
  stop_time_us = micros();
  if (VERBOSE)
    print_elapsed_time("I2C acquisition finished in ", start_time_us, stop_time_us);

  //acquire SPI data
  start_time_us = micros();
  channels.ch_spi = 0.0;  // to be defined
  {  // to be removed
    toggle_flag = !toggle_flag;
    if (toggle_flag) {
      spiWrite(128);
    }
    else {
      spiWrite(255);    
    }
  }
  stop_time_us = micros();
  if (VERBOSE)
    print_elapsed_time("SPI acquisition finished in ", start_time_us, stop_time_us);

  //STEP 3: retrieve timestamp and prepare data
  byte data;

  start_time_us = micros();
  txdata.cmd = MSG_ID_DATA;
  txdata.board_id = config_data.board_id;
  txdata.board_type = config_data.board_type;

  data = ReadI2CByte(RTCC_ADDR, 6); // get year from rtc
  txdata.yy = data & 0xff >> (0);
  data = ReadI2CByte(RTCC_ADDR, 5); // get month from rtc
  txdata.mth = data & 0xff >> (3);
  data = ReadI2CByte(RTCC_ADDR, 4); // get day from rtc
  txdata.dd = data & 0xff >> (2);

  data = ReadI2CByte(RTCC_ADDR, 2); // get hours from rtc
  txdata.hh = data & 0xff >> (2);
  data = ReadI2CByte(RTCC_ADDR, 1); // get minutes from rtc
  txdata.mm = data & 0xff >> (1);
  data = ReadI2CByte(RTCC_ADDR, 0); // get seconds from rtc
  txdata.ss = data & 0xff >> (1);

  { // to be removed
    int temp = 0x3F9E0652;
    achannels[0] = *((float *) (&temp)); //1.2345678806304931640625
  }
  int *pint = NULL;
  for (int i = 0; i < LEN_CHANNELS; i++) {
    //store the 4 bytes of the float number consecutively
    pint = (int *) &achannels[i];
    // byte order is big endian
    atxdata[CH_START_INDEX + 4 * i + 0] = (byte) ((*pint & 0xFF000000) >> 24);
    atxdata[CH_START_INDEX + 4 * i + 1] = (byte) ((*pint & 0x00FF0000) >> 16);
    atxdata[CH_START_INDEX + 4 * i + 2] = (byte) ((*pint & 0x0000FF00) >> 8);
    atxdata[CH_START_INDEX + 4 * i + 3] = (byte) ((*pint & 0x000000FF) >> 0);
  }

  stop_time_us = micros();
  if (VERBOSE)
    print_elapsed_time("data preparing finished in ", start_time_us, stop_time_us);

  //STEP 4: send TxData to broker
  start_time_us = micros();
  //... send LEN_TXDATA bytes
  if (VERBOSE) {
    for (int i = 0; i < LEN_TXDATA - 1; i++) {
      snprintf(mybuffer, sizeof(mybuffer), "0x%02x, ", atxdata[i]);
      Serial.print(mybuffer);
    }
    snprintf(mybuffer, sizeof(mybuffer), "0x%02x\n", atxdata[LEN_TXDATA - 1]);
    Serial.print(mybuffer);
    delayMicroseconds(10);
  }
  stop_time_us = micros();
  if (VERBOSE)
    print_elapsed_time("sending msg finished in ", start_time_us, stop_time_us);

  //print total time
  stop_time_us = micros();
  if (VERBOSE) {
    print_elapsed_time("loop iteration finished in ", initial_time_us, stop_time_us);
    Serial.println();
  }
  
  //wait delta time
  long deltaT = TLOOP_US - (micros() - initial_time_us);
  if (deltaT > 0) {
    delayMicroseconds(deltaT);
  }
  else {
    snprintf(mybuffer, sizeof(mybuffer), "cannot finish loop within %d us. system halted.", TLOOP_US);
    Serial.println(mybuffer);
    delayMicroseconds(10);
    while(1);   
  }
}

