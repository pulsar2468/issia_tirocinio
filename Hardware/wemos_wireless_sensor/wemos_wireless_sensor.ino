#include "ws_support_fcns.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include "DHT.h"
DHT dht(PIN_1WIRE, DHT22);


WiFiClient espClient;
PubSubClient client(espClient);

//global variables
//default wemos parameters
static const byte cmd = 99; //togliere
static const byte board_id = 33;
static const byte board_type = 0xF0;

//default wifi credentials
//max length for ssid and pwd is 124 chars + null
static const char* const wifi_ssid = "test1";
static const char* const wifi_pwd = "magic";

//default broker credentials
//max length for user and pwd is 124 chars + null
static const char* const mqtt_user = "issia";
static const char* const mqtt_pwd = "cnr";
static char mqtt_addr[16] = "150.145.127.37";
static unsigned int mqtt_port = 8883;

//other global variables
static struct config_data_t config_data;
static byte *aconfig_data = (byte *) &config_data; //alias of config_data as an array
static char mybuffer[200]; //a buffer to build formatted strings
static byte rxdata[EEPROM_SIZE + 8];
static unsigned int rxdatalen;

String tmp = "";


//*****************************************************************************
// retrieve new messages
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  int i;

  if (VERBOSE) {
    Serial.print("msg received on topic ");
    Serial.print(topic);
    Serial.println(":");
  }

  unsigned int min_length = (length + 1) < sizeof(rxdata) ? (length + 1) : sizeof(rxdata);
  snprintf((char *) rxdata, min_length, "%s", (char *) payload);
  rxdatalen = min_length - 1;

  if (VERBOSE) {
    for (i = 0; i < rxdatalen; i++) {
      snprintf(mybuffer, sizeof(mybuffer), "0x%02x, ", rxdata[i]);
      Serial.print(mybuffer);
      if ((i != 0) && (i % 32 == 31)) Serial.println();
    }
    if (i % 32 != 0) Serial.println();
  }
}

//*****************************************************************************

void setup_wifi(char *wifi_ssid, char *wifi_pwd) {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_pwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected - ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//*****************************************************************************
//binding to broker mosquitto
void mqtt_reconnect(char *clientID, char *username, char *pwd) {  // check 1st datatype
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection... ");
    if ((DEBUG_FAKE_BROKER) || client.connect(clientID, username, pwd)) {
      Serial.println("connected");
      return;
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Will try again in 5 seconds");
      delay(5000);
    }
  }
}

//*****************************************************************************

void setup()
{
  //setup serial port
  Serial.begin(115200);

  //setup GPIO pins
  pinMode(PIN_MUX_S0, OUTPUT);
  pinMode(PIN_MUX_S1, OUTPUT);
  pinMode(PIN_MUX_S2, OUTPUT);

  //setup 1-Wire
  dht.begin();
  // it sets pinMode(PIN_1WIRE, INPUT_PULLUP)
  // built in led is on same pin, so we must set
  // pinMode(PIN_LED, OUTPUT) only when we want
  // to flash the LED, and restore it afterwards

  //setup I2C
  Wire.begin();

  //setup SPI master
  //  pinMode(PIN_SPI_SCLK, OUTPUT);
  //  pinMode(PIN_SPI_MISO, INPUT);
  //  pinMode(PIN_SPI_MOSI, OUTPUT);
  pinMode(PIN_SPI_SSn, OUTPUT);
  SPI.begin();

  //initialize config_data fields at runtime, if required
  if ((PROGRAM_EEPROM == 1) || (DEBUG_FAKE_EEPROM == 1)) {
    //internal variables
    {
      byte MQTT_IPaddr_3;
      byte MQTT_IPaddr_2;
      byte MQTT_IPaddr_1;
      byte MQTT_IPaddr_0;
      byte MQTT_port_1;
      byte MQTT_port_0;
      byte MQTT_user_len;
      byte MQTT_pwd_len;
      byte wifi_ssid_len;
      byte wifi_pwd_len;

      //build parameters at runtime
      MQTT_IPaddr_3 = 150; //TODO: to be obtained with regexpr starting from mqtt_addr
      MQTT_IPaddr_2 = 145;
      MQTT_IPaddr_1 = 127;
      MQTT_IPaddr_0 = 37;
      MQTT_port_1 = (mqtt_port & 0x0000FF00) >> 8;
      MQTT_port_0 = (mqtt_port & 0x000000FF);
      MQTT_user_len = strlen(mqtt_user);
      MQTT_pwd_len = strlen(mqtt_pwd);
      wifi_ssid_len = strlen(wifi_ssid);
      wifi_pwd_len = strlen(wifi_pwd);

      //fill fixed-size config_data fields
      config_data.board_id = board_id;
      config_data.board_type = board_type;
      config_data.MQTT_IPaddr_3 = MQTT_IPaddr_3;
      config_data.MQTT_IPaddr_2 = MQTT_IPaddr_2;
      config_data.MQTT_IPaddr_1 = MQTT_IPaddr_1;
      config_data.MQTT_IPaddr_0 = MQTT_IPaddr_0;
      config_data.MQTT_port_1 = MQTT_port_1;
      config_data.MQTT_port_0 = MQTT_port_0;

      //fill variable-size config_data fields, zeroing excess chars
      int index = EEPROM_VAR_START_IDX;
      aconfig_data[index++] = MQTT_user_len;
      for (int i = 0; i < EEPROM_STR_SIZE; i++) {
        if (i >= MQTT_user_len)
          aconfig_data[index++] = 0;
        else
          aconfig_data[index++] = mqtt_user[i];
      }
      aconfig_data[index++] = MQTT_pwd_len;
      for (int i = 0; i < EEPROM_STR_SIZE; i++) {
        if (i >= MQTT_pwd_len)
          aconfig_data[index++] = 0;
        else
          aconfig_data[index++] = mqtt_pwd[i];
      }
      aconfig_data[index++] = wifi_ssid_len;
      for (int i = 0; i < EEPROM_STR_SIZE; i++) {
        if (i >= wifi_ssid_len)
          aconfig_data[index++] = 0;
        else
          aconfig_data[index++] = wifi_ssid[i];
      }
      aconfig_data[index++] = wifi_pwd_len;
      for (int i = 0; i < EEPROM_STR_SIZE; i++) {
        if (i >= wifi_pwd_len)
          aconfig_data[index++] = 0;
        else
          aconfig_data[index++] = wifi_pwd[i];
      }
    }
  }

  //if we only want to program the eeprom...
  if (PROGRAM_EEPROM) {
    //program config_data in eeprom
    if (program_eeprom(&config_data)) {
      toggle_confirmation_led(PIN_LED); // do not return
    }
    else {
      toggle_error_led(PIN_LED); // do not return
    }
  }

  //otherwise read config data from eeprom, if not in debug mode
  if (!DEBUG_FAKE_EEPROM) {
    //retrieve config data from eeprom
    //read_config_data_from_eeprom(&config_data); //TODO:
  }

  //check for correctness of eeprom data
  if ((config_data.board_type != 0xF0) && (config_data.board_type != 0xFE)) {
    Serial.println("Unknown board type or empty EEPROM!");
    toggle_error_led(PIN_LED); // do not return
  }

  if (!DEBUG_FAKE_EEPROM) {
    //overwrite default mqtt_addr and mqtt_port (global variables)
    snprintf(mqtt_addr, sizeof(mqtt_addr), "%u.%u.%u.%u", config_data.MQTT_IPaddr_3, config_data.MQTT_IPaddr_2,
             config_data.MQTT_IPaddr_1, config_data.MQTT_IPaddr_0);
    mqtt_port = (config_data.MQTT_port_1 << 8) + config_data.MQTT_port_0;
  }

  //print all credentials
  if (VERBOSE) {
    snprintf(mybuffer, sizeof(mybuffer), "SSID: %s, pwd: %s, Broker addr: %s, port: %u, user: %s, pwd: %s",
             config_data.WiFi_SSID, config_data.WiFi_pwd, mqtt_addr, mqtt_port,
             config_data.MQTT_user, config_data.MQTT_pwd);
    Serial.println(mybuffer);
  }

  //setup WiFi connection
  WiFi.mode(WIFI_STA); // only station, it doesn't initialize an AP!
  setup_wifi(config_data.WiFi_SSID, config_data.WiFi_pwd);

  //setup MQTT server
  client.setServer(mqtt_addr, mqtt_port);
  client.setCallback(mqtt_callback);

  //connect to broker
  mqtt_reconnect("WemosClient", config_data.MQTT_user, config_data.MQTT_pwd); //tutte le Wemos hanno lo stesso id?

  //send welcome msg and subscribe to config manager's topic
  snprintf(mybuffer, sizeof(mybuffer), "wemos_%u_%u", config_data.board_id, config_data.board_type);
  client.publish("/welcome", mybuffer); //topic, payload
  Serial.println("Welcome message sent");
  client.subscribe("/requestWelcomeToServer"); //config data TODO: cambiare nome
  client.subscribe("/request"); //TODO: quale dei due?

  Serial.println("End of setup");
  Serial.println();
}

//*****************************************************************************

void loop() {
  static bool toggle_flag = false;
  static unsigned long initial_time_us, start_time_us, stop_time_us;

  static struct channels_t channels;
  static float *achannels = (float *) &channels; // alias of channels as an array
  static struct txdata_t txdata;
  static byte *atxdata = (byte *) &txdata; //alias of txdata as an array

  initial_time_us = micros();
  if (VERBOSE) Serial.println("starting loop...");
  delayMicroseconds(10);

  //STEP 1: process new messages
  client.loop();
  if (DEBUG_FAKE_MSG && toggle_flag) {
    mqtt_callback("Prova", (byte *) "abcdefghijklmnopqrstuvwxyz789012\xB7\xF9", 34);
  }

  if (rxdatalen != 0) { // if msg arrived
    if (rxdata[0] == MSG_ID_WHO_ARE_YOU) {
      snprintf(mybuffer, sizeof(mybuffer), "wemos_%u_%u", config_data.board_id, config_data.board_type);
      client.publish("/welcome", mybuffer); //topic, payload
      Serial.println("Welcome message sent");
    }
    else if (rxdata[0] == MSG_ID_CONFIG) {
      start_time_us = micros();
      //program RTCC
      //    WriteI2CByte(RTCC_ADDR, 0x00, 0x00);    //stop RTCC and reset seconds
      //    WriteI2CByte(RTCC_ADDR, 0x06, config_data.yy);
      //    WriteI2CByte(RTCC_ADDR, 0x05, config_data.mth);
      //    WriteI2CByte(RTCC_ADDR, 0x04, config_data.dd);
      //    WriteI2CByte(RTCC_ADDR, 0x02, config_data.hh);
      //    WriteI2CByte(RTCC_ADDR, 0x01, config_data.mm);
      //    WriteI2CByte(RTCC_ADDR, 0x00, config_data.ss | 0x80);  //write seconds and start RTCC
      Serial.println("RTCC programmed");
      delayMicroseconds(10);

      //store config_data in EEPROM
      //    if (!program_eeprom(&config_data)) {
      //      toggle_error_led(PIN_LED); // do not return
      //    }

      stop_time_us = micros();
      print_elapsed_time("msg processing finished in ", start_time_us, stop_time_us);
    }
    else {
      Serial.println("Unknown message!");
    }
    rxdatalen = 0; // mark the msg as read
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
  { // to be removed
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
  //txdata.cmd = MSG_ID_DATA;
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
  txdata.ch_a0 = 1.2345; // to be removed
  stop_time_us = micros();
  if (VERBOSE)
    print_elapsed_time("data preparing finished in ", start_time_us, stop_time_us);

  //STEP 4: send TxData to broker
  start_time_us = micros();
  //... send LEN_TXDATA bytes
  if (VERBOSE) {
    for (int i = 0; i < LEN_TXDATA - 1; i++) {
      //snprintf(mybuffer, sizeof(mybuffer), "0x%02x, ", atxdata[i]);
      //Serial.println(atxdata[i],HEX);
      //      if (i>2 && i<9) tmp+=String(atxdata[i],HEX);
      //      else tmp+= (atxdata[i]);
      //tmp+=" ";
      tmp += atxdata[i];
    }
    //snprintf(mybuffer, sizeof(mybuffer), "0x%02x\n", atxdata[LEN_TXDATA - 1]);
    tmp += atxdata[LEN_TXDATA - 1];
    delayMicroseconds(10);
  }
  tmp[0] = 17;
  tmp[1] = 255;
  tmp[2] = 0x63;
  if (!client.connected()) {
    mqtt_reconnect(0, 0, 0); // reconnect to server mqtt
  }
  tmp.toCharArray(mybuffer, 200);
  //Serial.println(mybuffer);
  client.publish("wemos0/data", (byte *) &txdata, LEN_TXDATA);
  tmp = "";
  Serial.println(mybuffer);
  memset(mybuffer, 0, 200);
  delayMicroseconds(10);
  stop_time_us = micros();
  if (VERBOSE)
    print_elapsed_time("sending msg finished in ", start_time_us, stop_time_us);

  //print total time
  stop_time_us = micros();
  print_elapsed_time("loop iteration finished in ", initial_time_us, stop_time_us);
  Serial.println();

  //wait delta time
  long deltaT = TLOOP_US - (micros() - initial_time_us);
  if (deltaT > 0) {
    delayMicroseconds(deltaT);
  }
  else {
    //snprintf(mybuffer, sizeof(mybuffer), "cannot finish loop within %d us. system halted.", TLOOP_US);
    //Serial.println(mybuffer);
    delayMicroseconds(10);
    //while(1);
  }
}


