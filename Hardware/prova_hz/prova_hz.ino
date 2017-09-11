#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
double finalMillis;
double initialMillis = 0;
int count = 0;
double sum=0.0;
double a[1000];
int i;
int point=1000;
double value_m;
double value_e;
double sum_square=0.0;
const char* ssid = "test1";
const char* password = "magic";
HTTPClient http;
WiFiServer server(8001);

void setup() {
  Serial.begin(115200);
  //initialMillis = micros();
  delay(10);
  // Connect to WiFi network
  Serial.println();
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
  
  // Start the server
  server.begin();
  Serial.println("Server started");
  // Print the IP address
  Serial.println(WiFi.localIP());
   
}


void post_to_server(double value_m,double value_e) {
http.begin("http://10.42.0.1:8000/weather/DataFromWs/?data="+String(value_m)+","+String(value_e));
http.addHeader("Content-Type", "text/plain");
initialMillis=millis();
int httpCode = http.GET();   //Send the request
Serial.println(millis() - initialMillis);
//String payload = http.getString();                                        //Get the response payload

//Serial.println("Response Code:" + String(httpCode));   //Print HTTP return code
//Serial.println(payload);    //Print request response payload
http.end();  //Close connection
}


void loop() {
  
  //for (;;) {
    for (i=0;i<point;i++) {
      a[i]=(analogRead(A0)*3.2*1.0538)/1024;
      sum+=a[i];
      delayMicroseconds(100);
    }


    value_m=sum/point;

    for (i=0;i<point;i++) {
        sum_square+=pow(a[i]-value_m,2);
    }
    
    value_e=sqrt(sum_square/point);
    //Serial.println(value_m);
    //Serial.println(value_e);
    sum=0;
    sum_square=0;
    post_to_server(value_m,value_e);
    delay(3000);
    
    /*
    for (i=0;i<point;i++) {
      Serial.println(a[i]);
      delayMicroseconds(100);
    }
    */

    /*
    finalMillis = micros();
    if (finalMillis - initialMillis <= 500) {
      count++;
    }
    else {
     Serial.println(count);
      count = 0;
     delay(2000);
     initialMillis = micros();
    }
  }
  */
}
