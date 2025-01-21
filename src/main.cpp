#include <Arduino.h>
void setup() {
  Serial.begin(115200);
  pinMode(48, OUTPUT);
  digitalWrite(48, LOW);

  while (true) {
    digitalWrite(48, HIGH);
    delay(500);
    digitalWrite(48, LOW);
    delay(500);
  }
}

void loop() {
  // write your code here
}
