unsigned int count = 0;


void timer0_ISR (void){
  Serial.println(count);
  count=0;
  timer0_detachInterrupt();
  //timer0_write(ESP.getCycleCount() + 80000000); //80Mhz -> 80*10^6 = 1 second
}

void setup() {
  Serial.begin(115200);
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(timer0_ISR);
  timer0_write(ESP.getCycleCount() + 80000000); //80Mhz -> 80*10^6 = 1 second
  interrupts();
}


void loop() {
  get_data();
  /*
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
    Serial.println(value_m);
    Serial.println(value_e);
    sum=0;
    sum_square=0;
    //post_to_server(value_m,value_e);
    delay(3000);
    
    
    for (i=0;i<point;i++) {
      Serial.println(a[i]);
      delayMicroseconds(100);
    }
    */
  
  
  
}
