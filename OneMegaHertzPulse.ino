/**
 * OneMegaHertzPulse - Sets up a 1 MHz pulse train on pins 9 and 10
 */
void setup() {
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  TCCR1A = _BV(COM1A0) | _BV(COM1B0); // toggle OC1A/B on Compare Match
  TCCR1B = _BV(WGM12) | _BV(CS10); // CTC, no prescaler
  OCR1A = 7; // 1 MHz

}

void loop() {
  // put your main code here, to run repeatedly:

}
