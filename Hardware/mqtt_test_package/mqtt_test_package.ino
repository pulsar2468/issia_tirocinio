#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

double initialMillis=0,finalMillis;
const char* ssid = "test1";
const char* password = "magic";
const char* mqtt_server = "10.42.0.1";
int packet=1000;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[5];
int value = 0;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 8883);
  //client.setCallback(callback);
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient", "pulsar", "conoscenza")) {
      Serial.println("connected");
      // Once connected, publish an announcement...

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

  if (!client.connected()) {
    reconnect();
  }

  double start=millis();
  while(packet>0){
  long now = millis();
  if (now - lastMsg > 0) {
    lastMsg = now;
    //Serial.print("Publish message: ");
    //msg=random(100)+" "+random(100);
    snprintf (msg, 5, "%d %d", random(100),random(100));
    //Serial.println(msg);
    initialMillis=millis();
    client.publish("wemos0/dht11", msg,0);
    Serial.println(millis() - initialMillis);
    packet--;
    //client.disconnect();
  }
  }
  Serial.println((millis() - start)/1000);
  delay(100000);
}
