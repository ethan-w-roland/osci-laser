int num_bits = 50;
byte buf[50] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0};
int i = 0;

void setup() {

  //setup pwm out
  pinMode(8, OUTPUT);
  
  cli(); //disable intrps
  
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 5340;// = int((16*10^6) / bit_freq) - 1 (must be <65536)
  TCCR1B |= (1 << WGM12);
  TCCR1B |= 1; //set prescaler to 1
  TIMSK1 |= (1 << OCIE1A);
  
  sei(); //enable intrps
}

ISR(TIMER1_COMPA_vect) {
  PORTB =  buf[i];
  i = i + 1;
  i = i % num_bits;
}

void loop() {
  //nothing in particular
  while(1) {
    delay(2000);
    Serial.println("ping1");
    buf[0] = 1;
    buf[1] = 1;
    buf[2] = 1;
    buf[3] = 1;
    delay(2000);
    Serial.println("ping2");
    buf[0] = 1;
    buf[1] = 0;
    buf[2] = 1;
    buf[3] = 0;
  }
}
