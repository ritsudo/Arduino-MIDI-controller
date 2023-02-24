unsigned int adcSum = 0;
unsigned char adcOut;

unsigned char c0 = 0;
unsigned char c1 = 0;
unsigned char c2 = 0;

void setup() {
  Serial.begin(57600);
}

void loop() {
  adcSum = 0;

  for (int i = 0; i < 3; i++) {
    adcSum += analogRead(0)/8;
  }
  adcOut = adcSum/3;
  c2 = adcOut;

  if (c2-c1 != 0) {
    if (c2 != c0) {
      Serial.write(c2);
      c0 = c1;
      c1 = c2;
    }
  }
  
}
