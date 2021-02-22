#include <Wire.h>

#define DacAddress byte(0x60)
#define DacWriteCmd int(64)
#define SensorAddress byte(0x70)
#define RangeCommand byte(0x51)

const int num_bits = 50;
const int range_low = 20;
const float range_high = 160.0;

byte buf[50] = {1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0};
int i = 0;

void setup() {

  //setup i2c
  Serial.begin(9600);
  Wire.begin();

  //setup buf out
  pinMode(8, OUTPUT);

  //setup gate out
  pinMode(0, OUTPUT);
  digitalWrite(0, LOW);
  
  cli(); //disable intrps
  
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 5341;// = int((16*10^6) / bit_freq) - 1 (must be <65536)
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

float old_per = 0;
float per = 0;
void loop() {
  
  word range = requestRange();
  Serial.print("R:");
  Serial.println(range);
  takeRangeReading();

  int ind;
  if (range <= range_high & range >= range_low) {
    per = float(range-range_low) / (range_high-range_low);
    int clock_change = int((per - 0.5)*-90.0);
    OCR1A = 5341 + clock_change;
  }

  //Serial.println(per);

  int dac_val;
  if (per != old_per) {
    digitalWrite(0, HIGH);
    dac_val = int( per * 4096.0 );
    setDacVal(dac_val);
    digitalWrite(0, LOW);
  }
  old_per = per;
  delay(150);
}

void setDacVal(int dac_val)
{
  Wire.beginTransmission(DacAddress);
  Wire.write(DacWriteCmd);
  Wire.write(dac_val>>4);
  Wire.write(dac_val<<4);
  Wire.endTransmission();
}

void takeRangeReading()
{
  Wire.beginTransmission(SensorAddress);
  Wire.write(RangeCommand);
  Wire.endTransmission();
}

word requestRange() {
  Wire.requestFrom(SensorAddress, byte(2));
  if (Wire.available() >= 2)
  {
    byte high_byte = Wire.read();
    byte low_byte = Wire.read();
    word range = word(high_byte, low_byte);
    return range;
  }
  else
  {
    return word(0);
  }
}
