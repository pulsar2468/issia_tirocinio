#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//web-server and broker mqtt
const char* ssid = "test1";
const char* password = "magic";
const char* mqtt_server = "10.42.0.1";
WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
String output_value;

//logical computational
double finalMillis;
double initialMillis = 0;
int count = 0;
double sum = 0.0, f;
double v[1000];
double current[1000], p[1000];
double id[2];
int i;
double T = 0.0, sum_t;
int point = 1000;
double value_m, value_m_of_p;
double value_e, value_a, value_q;
double sum_square = 0.0, delta_t = 0.0;
int flag = 0;
double v_ac[1000];
int signs[1000];
#define t_sample_us 500
String data_string;

//wave frequency
void frequency() {

  for (i = 0; i < point; i++) {
    //signs[i] = not(v_ac[i] > 0.1 or v_ac[i] < -0.1);
    signs[i] = (v_ac[i] >= 0  ? 1 : 0);
  }
  for (i = 0; i < point; i++) {
    if (flag == 2) {
      flag = 0;
      return;
    }
    if (signs[i] == 0) {
      if (signs[i + 1] == 1) {               
        id[flag] =  v_ac[i+1]/(v_ac[i]-v_ac[i+1]) +i+1;
        flag++;
      }
    }
  }
}


void setup() {
  //pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 8883); //set Server mqtt on port 8883
  //client.setCallback(callback);
  Serial.print(output_value);
  delay(100000);
  }


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


//bind to broker mosquitto
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("WemosClient", "pulsar", "conoscenza")) {
      Serial.println("connected");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {

  ESP.wdtEnable(0); // disable watchdog to prevent wdt.rest for false deadlock

  initialMillis=ESP.getCycleCount();

  //To get analog read
  for (i = 0; i < point; i++) {
    delta_t = ESP.getCycleCount() - initialMillis;
    delayMicroseconds(t_sample_us - (delta_t/150.45));
    initialMillis=ESP.getCycleCount();
    v[i] = (analogRead(A0));
  }


  for (i = 0; i < point; i++) {
    v[i] = (v[i] * 3.2 * 1.0538) / 1024; //mapping values into voltage space (axis y)
    sum_square += pow(v[i] - value_m, 2);
    current[i] = v[i]; //assignment temporany
    p[i] = v[i] * current[i]; //potenza istantanea
  }


  for (i = 0; i < point; i++) {
    sum += v[i];
  }

  value_m = sum / point;
  for (i = 0; i < point; i++) {
    v_ac[i] = v[i] - value_m; //
  }

  value_e = sqrt(sum_square / point);

  //medium value of p
  for (i = 0; i < point; i++) {
  sum += p[i];
  }

  value_m_of_p = sum / point;
  value_a = value_e * value_e; //efficient value v * efficient value current
  value_q = sqrt(pow(value_a, 2) - value_m_of_p );

  frequency();
  T = (id[1] - id [0]) * (t_sample_us) * 1e-6;
  //Serial.println(T,5);
  output_value = "Frequency: " + String(1.0/T,6) + '\n' + "Periodo: " + String(T,6) + '\n'
  + "Average_value: " + String(value_m,6) + '\n' + + "Effective_value: " + String(value_e,6) +
  "\n\n\n";
  Serial.print(output_value);

  //clear variables
  sum = 0;
  sum_square = 0;

  //if (!client.connected()) { 
  //  reconnect(); // reconnect to server mqtt
  //}

  //client.publish("wemos0/dht11", msg,0); //publish value into mqtt broker
  delay(1);
}
