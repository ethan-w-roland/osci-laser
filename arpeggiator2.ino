#include <Wire.h>

#define DacAddress byte(0x60)
#define DacWriteCmd int(64)
#define SensorAddress byte(0x70)
#define RangeCommand byte(0x51)

const int num_bits = 50;
const int range_low = 40;
const float range_high = 150.0;

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

float per = 0;
int num_notes = 3;
float notes[3] = {0};
float old_range = 0;
unsigned long old_time = 0;
int new_time = 0;
int cur_note = 0;
int dac_val;
int ind;


void loop() {

  //get sensor val
  word range = requestRange();
  Serial.print("R:");
  Serial.println(range);
  takeRangeReading();

  //calc percentage
  if (range <= range_high & range >= range_low) {
    per = float(range-range_low) / (range_high-range_low);
  }

  //update notes[]
  if (range > range_high & old_range < range_high) { //falling edge
    //cycle notes
    for (int i = 0; i < num_notes; i++) {
      notes[i] = notes[i+1];
    }
    //write to end
    notes[num_notes-1] = per;
  }

  new_time = millis();
  if (new_time - old_time > 300){
    old_time = new_time;
    //write DAC
    digitalWrite(0, HIGH);
    dac_val = int( notes[cur_note] * 4096.0 );
    setDacVal(dac_val);
    digitalWrite(0, LOW);
    //phase shift
    int clock_change = int((notes[cur_note] - 0.5)*-200.0);
    OCR1A = 5341 + clock_change;
    cur_note = cur_note + 1;
    cur_note = cur_note % num_notes;
  }

  old_range = range;
  delay(100);
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
