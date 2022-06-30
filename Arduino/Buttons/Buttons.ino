void setup() {
  // put your setup code here, to run once:
  pinMode(0, INPUT);
  digitalWrite(0, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(0) == LOW) {
    Serial.println("Button is pressed");
  }

}
