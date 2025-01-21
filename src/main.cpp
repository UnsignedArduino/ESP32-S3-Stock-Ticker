#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <MD_Parola.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 16

#define CLK_PIN 11
#define DATA_PIN 13
#define CS_PIN 12

MD_Parola p = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup() {
  Serial.begin(115200);
  pinMode(48, OUTPUT);
  digitalWrite(48, LOW);

  p.begin();
  p.setIntensity(1);
  p.setSpeed(20);

  while (true) {
    if (p.displayAnimate()) {
      p.displayText("Hello, World! Here is a really long message to test "
                    "scrolling text.",
                    PA_LEFT, p.getSpeed(), 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    }
  }
}

void loop() {
  // Using while loop in setup to mimic loop because actually using loop crashes
}
