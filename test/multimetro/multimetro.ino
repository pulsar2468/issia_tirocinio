double finalMillis;
double initialMillis = 0;
int count = 0;
double sum = 0.0, f;
double v[1000], current[1000], p[1000];
int id[2];
int i;
double T = 0.0;
int point = 1000;
double value_m, value_m_of_p;
double value_e, value_a, value_q;
double sum_square = 0.0;
int flag = 0;
double v_ac[1000];
double signs[1000];
#define t_sample_us 100 

void frequency() {

  for (i = 0; i < point; i++) {
    if (v_ac[i] < - 0.008) signs[i] = 0;
    else signs[i]=1;
    delay(1);
  }


  /*
    if (flag == 2) {
     flag = 0;
     return;
    }

    if (v[i] < 0) {
     if (v[i + 1] > 0) {
       id[flag] = i + 1;
       flag++;
     }
    }

    else if (v[i] > 0) {
     if (v[i + 1] < 0) {
       id[flag] = i + 1;
       flag++;
     }
    }
  */
  for (i = 0; i < point; i++) {

    if (flag == 2) {
      flag = 0;
      return;
    }
    if (signs[i] == 0) {
      if (signs[i + 1] == 1) {
        id[flag] = i + 1;
        flag++;
      }
    }
    //delay(1);
  }
}



void setup() {
  Serial.begin(115200);
}


void loop() {

  //for (;;) {
  for (i = 0; i < point; i++) {
    v[i] = (analogRead(A0) * 3.2 * 1.0538) / 1024;
    Serial.println(v_ac[i]);
    sum += v[i];
    delayMicroseconds(t_sample_us);
  }


  value_m = sum / point;

  for (i = 0; i < point; i++) {
    sum_square += pow(v[i] - value_m, 2);
  }

  for (i = 0; i < point; i++) {
    v_ac[i] = v[i] - value_m;
  }

  /*
    value_e = sqrt(sum_square / point);
    for (i = 0; i < point; i++) {
      current[i] = v[i]; //assignment temporany
    }

    //potenza istant.
    for (i = 0; i < point; i++) {
      p[1000] = v[i] * current[i];
    }

    //medium value of p
    for (i = 0; i < point; i++) {
      sum += p[i];
    }
    value_m_of_p = sum / point;

    value_a = value_e * value_e; //efficient value v * efficient value current
    value_q = sqrt(pow(value_a, 2) - value_m_of_p );
  */





  frequency();
  T = (id[1] - id [0]) * t_sample_us*1e-6;
  //Serial.println(value_m); // of v
  //Serial.println(value_e); //of v
  Serial.println(T, 4); // print frequency
  Serial.println(1 / T);

  //clear variables
  sum = 0;
  sum_square = 0;
  delay(1000);

}
