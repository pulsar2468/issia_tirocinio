double finalMillis;
double initialMillis = 0;
int count = 0;
double sum = 0.0, f;
double v[1000];
//double current[1000], p[1000];
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

void frequency() {

  for (i = 0; i < point; i++) {
    //signs[i] = not(v_ac[i] > 0.1 or v_ac[i] < -0.1);
    signs[i] = (v_ac[i] >= 0  ? 1 : 0);
    //Serial.println(signs[i]);
  }

  for (i = 0; i < point; i++) {

    if (flag == 2) {
      flag = 0;
      return;
    }
    if (signs[i] == 0) {
      if (signs[i + 1] == 1) {       
        //id[flag] = i;
        
        id[flag] =  v_ac[i+1]/(v_ac[i]-v_ac[i+1]) +i+1;
        flag++;
      }
    }
  }
}



void setup() {
  Serial.begin(115200);
}


void loop() {

  //for (;;) {
  initialMillis = ESP.getCycleCount();
  //initialMillis = micros();
  for (i = 0; i < point; i++) {
    delta_t = ESP.getCycleCount() - initialMillis;
    //delta_t = micros() - initialMillis;


    if (delta_t <= 156) { //t_sample_us!
      //noInterrupts();
      delayMicroseconds(t_sample_us - delta_t);
      //initialMillis = micros();
      initialMillis=ESP.getCycleCount();
      v[i] = (analogRead(A0));
      //interrupts();
    }

    else  {
      Serial.println("task overrun");

      delay(1);
      //initialMillis = micros();
          initialMillis=ESP.getCycleCount();

    }

    

    //delayMicroseconds(t_sample_us);

  }





  for (i = 0; i < point; i++) {
    v[i] = (v[i] * 3.2 * 1.0538) / 1024;
    sum_square += pow(v[i] - value_m, 2);
    //current[i] = v[i]; //assignment temporany
    //p[i] = v[i] * current[i]; //potenza istantanea
  }


  for (i = 0; i < point; i++) {
    sum += v[i];
  }

  value_m = sum / point;
  for (i = 0; i < point; i++) {
    v_ac[i] = v[i] - value_m; //
  }

  //value_e = sqrt(sum_square / point);

  //medium value of p
  //for (i = 0; i < point; i++) {
  //sum += p[i];
  //}

  //value_m_of_p = sum / point;
  //value_a = value_e * value_e; //efficient value v * efficient value current
  //value_q = sqrt(pow(value_a, 2) - value_m_of_p );

  frequency();
  T = (id[1] - id [0]) * (t_sample_us) * 1e-6;
  Serial.println(T,5);
  Serial.print("Frequency: ");
  Serial.println(1.0 / T, 5);

  //clear variables
  sum = 0;
  sum_square = 0;
  //Serial.println(id[1]);
  //Serial.println(id[0]);
  Serial.print("Range: ");
  Serial.println(id[1] - id[0]);
  Serial.println((T / 0.01 -1)*100);
  //for (i=0;i<point;i++) if (id[1] - id[0]!=25 && id[0] > 25) Serial.println(v_ac[i]);

  
  //Serial.println(millis() - initialMillis);
}
