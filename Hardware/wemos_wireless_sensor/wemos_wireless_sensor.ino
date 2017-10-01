#include "ws_support_fcns.h"
#include <assert.h>
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
static const byte default_board_id = 33;
static const byte default_board_type = 0xF0;
//board_type = 0x00 means unprogrammed board

//default wifi credentials
static const char* const default_wifi_ssid = "test1";
static const char* const default_wifi_pwd = "magic";

//default broker credentials
static const char default_mqtt_addr[16] = "150.145.127.37";
static const unsigned int default_mqtt_port = 8883;
static const char* const default_mqtt_user = "issia";
static const char* const default_mqtt_pwd = "cnr";

//check length of default wifi ssid/pwd and mqtt user/pwd
constexpr unsigned int MQTT_user_len = strlen(default_mqtt_user);
constexpr unsigned int MQTT_pwd_len = strlen(default_mqtt_pwd);
constexpr unsigned int wifi_ssid_len = strlen(default_wifi_ssid);
constexpr unsigned int wifi_pwd_len = strlen(default_wifi_pwd);
static_assert((
        (MQTT_user_len + 1 <= CONFIG_DATA_STR_SIZE) &&
        (MQTT_pwd_len + 1 <= CONFIG_DATA_STR_SIZE) &&
        (wifi_ssid_len + 1 <= CONFIG_DATA_STR_SIZE) &&
        (wifi_pwd_len + 1 <= CONFIG_DATA_STR_SIZE) ),
        "Default wifi ssid/pwd and default mqtt user/pwd must contain no more "
        "than CONFIG_DATA_STR_SIZE chars, including trailing NULL. Can't compile.");

//other global variables
static char mqtt_addr[16];
static unsigned int mqtt_port;
static struct config_data_t config_data;
static byte *aconfig_data = (byte *) &config_data; //alias of config_data as an array
static char mybuffer[200]; //a buffer to build formatted strings
static byte rxdata[RXDATA_BUFSIZE];
static unsigned int rxdatalen;


//*****************************************************************************
// retrieve new messages
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  int i;

  Serial.print("msg received on topic ");
  Serial.println(topic);

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
    //populate config_data with default values...
    //fill fixed-size config_data fields
    config_data.board_id = default_board_id;
    config_data.board_type = default_board_type;
    splitIPaddress((char *) default_mqtt_addr, &config_data.MQTT_IPaddr_3, &config_data.MQTT_IPaddr_2,
                   &config_data.MQTT_IPaddr_1, &config_data.MQTT_IPaddr_0);
    splitIPport(default_mqtt_port, &config_data.MQTT_port_1, &config_data.MQTT_port_0);

    //fill strings in config_data fields, zeroing excess chars
    int index = CONFIG_DATA_STR_START_IDX;
    for (int i = 0; i < CONFIG_DATA_STR_SIZE; i++) {
      if (i > MQTT_user_len)
        aconfig_data[index++] = 0;
      else
        aconfig_data[index++] = default_mqtt_user[i];
    }
    for (int i = 0; i < CONFIG_DATA_STR_SIZE; i++) {
      if (i > MQTT_pwd_len)
        aconfig_data[index++] = 0;
      else
        aconfig_data[index++] = default_mqtt_pwd[i];
    }
    for (int i = 0; i < CONFIG_DATA_STR_SIZE; i++) {
      if (i > wifi_ssid_len)
        aconfig_data[index++] = 0;
      else
        aconfig_data[index++] = default_wifi_ssid[i];
    }
    for (int i = 0; i < CONFIG_DATA_STR_SIZE; i++) {
      if (i > wifi_pwd_len)
        aconfig_data[index++] = 0;
      else
        aconfig_data[index++] = default_wifi_pwd[i];
    }
  }

  //if we only want to program the eeprom...
  if (PROGRAM_EEPROM) {
    bool b = program_eeprom(aconfig_data);
    Serial.println("Please reset the board...");
    if (b) {
      toggle_confirmation_led(PIN_LED); // do not return
    }
    else {
      toggle_error_led(PIN_LED); // do not return
    }
  }

  //otherwise read config data from eeprom, if not in debug mode
  if (!DEBUG_FAKE_EEPROM) {
    //retrieve config data from eeprom
    read_config_data_from_eeprom(aconfig_data);
  }

  //test for correctness of eeprom data by checking board_type
  if ((config_data.board_type != 0xF0) && (config_data.board_type != 0xFE)) {
    Serial.println("Unknown board type or empty EEPROM!");
    toggle_error_led(PIN_LED); // do not return
  }

  //fill mqtt_addr and mqtt_port (global variables)
  buildIPaddress(mqtt_addr, config_data.MQTT_IPaddr_3, config_data.MQTT_IPaddr_2,
                 config_data.MQTT_IPaddr_1, config_data.MQTT_IPaddr_0);
  mqtt_port = buildIPport(config_data.MQTT_port_1, config_data.MQTT_port_0);

  //print all credentials
  if (VERBOSE) {
    snprintf(mybuffer, sizeof(mybuffer), "SSID: %s, pwd: %s, Broker addr: %s, port: %u, user: %s, pwd: %s",
             config_data.WiFi_SSID, config_data.WiFi_pwd,
             mqtt_addr, mqtt_port, config_data.MQTT_user, config_data.MQTT_pwd);
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
  Serial.print("Welcome message sent: ");
  Serial.println(mybuffer);
  client.subscribe("/requestWelcomeToServer"); //config data TODO: cambiare nome
  client.subscribe("/request"); //TODO: quale dei due?

  if (VERBOSE) Serial.println("setup completed");
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

  //STEP 1: process new messages
  client.loop();
  if (DEBUG_FAKE_MSG && toggle_flag) {
    mqtt_callback("Prova", (byte *) "\x68\xFF\x01", 3);
    //mqtt_callback("Prova", (byte *) "\x63\x21\x12""defghijklmnopqrstuvwxyz789012\xB7\xF9", 34);
    //mqtt_callback("Prova", (byte *) "\x63\xFF\xFF""defghijklmnopqrstuvwxyz789012\xB7\xF9", 34);
    //mqtt_callback("Prova", (byte *) "\x05\x21\x17\x09\x30\x16\x45\x02", 8);
  }

  if ((rxdatalen != 0) && ((rxdata[1] == 0xFF) || (rxdata[1] == config_data.board_id))) {
    // if msg arrived and it is for this wemos or a broadcast msg
    start_time_us = micros();
    if (rxdata[0] == MSG_ID_WHO_ARE_YOU) {
      if (rxdata[2] == 0) {
        //short answer
        snprintf(mybuffer, sizeof(mybuffer), "wemos_%u_%u", config_data.board_id, config_data.board_type);
      }
      else {
        //long answer
        snprintf(mybuffer, sizeof(mybuffer), "wemos_%u_%u_%u", config_data.board_id, config_data.board_type, 0x01);
        //TODO: implement long answer
      }
      if (!client.connected()) {
        // reconnect to server mqtt
        mqtt_reconnect("WemosClient", config_data.MQTT_user, config_data.MQTT_pwd); //tutte le Wemos hanno lo stesso id?
      }
      client.publish("/welcome", mybuffer); //topic, payload
      Serial.print("Welcome message sent: ");
      Serial.println(mybuffer);
    }
    else if (rxdata[0] == MSG_ID_CONFIG) {
      //say goodbye to broker
      if (!client.connected()) {
        // reconnect to server mqtt
        mqtt_reconnect("WemosClient", config_data.MQTT_user, config_data.MQTT_pwd); //tutte le Wemos hanno lo stesso id?
      }
      //unsubscribe remote client topic /request?
      snprintf(mybuffer, sizeof(mybuffer), "wemos_%u_%u", config_data.board_id, config_data.board_type);
      client.publish("/goodbye", mybuffer); //topic, payload
      Serial.print("Goodbye message sent: ");
      Serial.println(mybuffer);

      //store config_data in EEPROM
      byte b;
      if (rxdata[1] == 0xFF) {
        b = config_data.board_id;
      }
      aconfig_data = &rxdata[2];
      if (rxdata[1] == 0xFF) {
        aconfig_data[0] = b;
      }
      program_eeprom(aconfig_data);
      Serial.println("Trying to reset the board...");
      ESP.restart();
    }
    else if (rxdata[0] == MSG_ID_SETDATETIME) {
      start_time_us = micros();
      //program RTCC
      WriteI2CByte(RTCC_ADDR, 0x00, 0x00);    //stop RTCC and reset seconds
      WriteI2CByte(RTCC_ADDR, 0x06, rxdata[2]);
      WriteI2CByte(RTCC_ADDR, 0x05, rxdata[3]);
      WriteI2CByte(RTCC_ADDR, 0x04, rxdata[4]);
      WriteI2CByte(RTCC_ADDR, 0x02, rxdata[5]);
      WriteI2CByte(RTCC_ADDR, 0x01, rxdata[6]);
      WriteI2CByte(RTCC_ADDR, 0x00, rxdata[7] | 0x80);  //write seconds and start RTCC
      Serial.println("RTCC programmed");
      Serial.println("current date/time:");
      timestamp();
    }
    else {
      Serial.println("Unknown msg id!");
    }
    stop_time_us = micros();
    if (PARTIAL_EXEC_TIME)
      print_elapsed_time("msg processing finished in ", start_time_us, stop_time_us);
  }
  rxdatalen = 0; // mark the msg as read

  //STEP 2: acquire data from hw sensors
  //acquire analog channels
  if (config_data.board_type == 0xFE) {
    //acquire V and I and compute electrical quantities
    start_time_us = micros();
    acquire_and_process_v_and_i(&channels);
    stop_time_us = micros();
    if (PARTIAL_EXEC_TIME)
      print_elapsed_time("electrical acquisition finished in ", start_time_us, stop_time_us);
  }
  else {
    //acquire raw analog signals
    start_time_us = micros();
    acquire_raw_analog_channels(&channels);
    stop_time_us = micros();
    if (PARTIAL_EXEC_TIME)
      print_elapsed_time("raw analog acquisition finished in ", start_time_us, stop_time_us);
  }
  if (VERBOSE) {
    Serial.println("Results:");
    Serial.println(channels.ch_a0, 6);
    Serial.println(channels.ch_a1, 6);
    Serial.println(channels.ch_a2, 6);
    Serial.println(channels.ch_a3, 6);
    Serial.println(channels.ch_a4, 6);
    Serial.println(channels.ch_a5, 6);
    Serial.println(channels.ch_a6, 6);
    Serial.println(channels.ch_a7, 6);
  }

  //acquire 1-Wire data
  start_time_us = micros();
  channels.ch_1wire = dht.readTemperature();
  stop_time_us = micros();
  if (PARTIAL_EXEC_TIME)
    print_elapsed_time("1-Wire acquisition finished in ", start_time_us, stop_time_us);

  //acquire I2C data
  start_time_us = micros();
  channels.ch_i2c = ReadI2CByte(I2C_SENSOR_ADDR, I2C_SENSOR_REG);
  stop_time_us = micros();
  if (PARTIAL_EXEC_TIME)
    print_elapsed_time("I2C acquisition finished in ", start_time_us, stop_time_us);

  //acquire SPI data
  start_time_us = micros();
  channels.ch_spi = 0.0;  //TODO: to be defined
  { // to be removed
    toggle_flag = !toggle_flag;
    if (toggle_flag) {
      WriteSPIByte(128);
    }
    else {
      WriteSPIByte(255);
    }
  }
  stop_time_us = micros();
  if (PARTIAL_EXEC_TIME)
    print_elapsed_time("SPI acquisition finished in ", start_time_us, stop_time_us);

  //STEP 3: retrieve timestamp and prepare data
  byte data;

  start_time_us = micros();
  txdata.msg_id = MSG_ID_NEWDATA;
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

  int *pint = NULL;
  for (int i = 0; i < CHANNELS_LEN; i++) {
    //store the 4 bytes of the float number consecutively
    //byte order is big endian
    pint = (int *) &achannels[i];
    atxdata[TXDATA_CH_START_IDX + 4 * i + 0] = (byte) ((*pint & 0xFF000000) >> 24);
    atxdata[TXDATA_CH_START_IDX + 4 * i + 1] = (byte) ((*pint & 0x00FF0000) >> 16);
    atxdata[TXDATA_CH_START_IDX + 4 * i + 2] = (byte) ((*pint & 0x0000FF00) >> 8);
    atxdata[TXDATA_CH_START_IDX + 4 * i + 3] = (byte) ((*pint & 0x000000FF) >> 0);
  }

  stop_time_us = micros();
  if (PARTIAL_EXEC_TIME)
    print_elapsed_time("data preparing finished in ", start_time_us, stop_time_us);

  if (VERBOSE) {
    int i;
    Serial.println("TXDATA:");
    for (i = 0; i < TXDATA_LEN; i++) {
      snprintf(mybuffer, sizeof(mybuffer), "0x%02x, ", atxdata[i]);
      Serial.print(mybuffer);
      if ((i != 0) && (i % 32 == 31)) Serial.println();
    }
    if (i % 32 != 0) Serial.println();
  }

  //STEP 4: send TxData to broker
  start_time_us = micros();
  if (!client.connected()) {
    // reconnect to server mqtt
    mqtt_reconnect("WemosClient", config_data.MQTT_user, config_data.MQTT_pwd); //tutte le Wemos hanno lo stesso id?
  }
  client.publish("wemos0/data", atxdata, TXDATA_LEN); // check topic
  Serial.println("data sent");
  stop_time_us = micros();
  if (PARTIAL_EXEC_TIME)
    print_elapsed_time("sending msg finished in ", start_time_us, stop_time_us);

  //print total time
  stop_time_us = micros();
  if (VERBOSE)
    print_elapsed_time("loop iteration took ", initial_time_us, stop_time_us);
  Serial.println();
  
  //wait delta time
  long deltaT = TLOOP_US - (micros() - initial_time_us);
  if (deltaT > 0) {
    delayMicroseconds(deltaT);
  }
  else {
    //say goodbye to broker
    if (!client.connected()) {
      // reconnect to server mqtt
      mqtt_reconnect("WemosClient", config_data.MQTT_user, config_data.MQTT_pwd); //tutte le Wemos hanno lo stesso id?
    }
    //unsubscribe remote client topic /request?
    snprintf(mybuffer, sizeof(mybuffer), "wemos_%u_%u", config_data.board_id, config_data.board_type);
    client.publish("/goodbye", mybuffer); //topic, payload
    Serial.print("Goodbye message sent: ");
    Serial.println(mybuffer);

    //alert console user
    snprintf(mybuffer, sizeof(mybuffer), "cannot finish loop within %d us. system halted.", TLOOP_US);
    Serial.println(mybuffer);
    toggle_error_led(PIN_LED); // do not return
  }
}

