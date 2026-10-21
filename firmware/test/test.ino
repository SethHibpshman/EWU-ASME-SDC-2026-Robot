void setup() {
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(13, HIGH);  // LED on
  delay(500);
  digitalWrite(13, LOW);   // LED off
  delay(500);
}