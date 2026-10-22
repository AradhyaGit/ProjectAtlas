#include <Servo.h>

Servo myServo;
const int STEP_DELAY = 20;
const int STEP_VALUE = 2;

void setup() {
  myServo.attach(9);
}

void moveStep(int angle) {
  myServo.write(angle);
  delay(STEP_DELAY);
}

void moveSmooth(int start, int end) {
  if(start < end) {
    for(int angle = start; angle <= end; angle += STEP_VALUE) {
      moveStep(angle);
    }
  }

  else {
    for(int angle = start; angle >= end; angle -= STEP_VALUE) {
      moveStep(angle);
    }
  }

}

void loop() {
  moveSmooth(0, 120);
  delay(2000);
  moveSmooth(120, 0);
  delay(2000);

}