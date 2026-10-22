#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);

void setup() {
  pca.begin();
  pca.setPWMFreq(50);
  delay(500);

  // Move channel 0 to approximately center
  pca.setPWM(4, 0, 207);
}

void loop() {
}