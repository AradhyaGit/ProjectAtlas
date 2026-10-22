#include <Servo.h>

Servo wrist;
Servo elbow;
const int STEP_DELAY = 20;
const int STEP_VALUE = 3;

void moveStep(Servo &servo, int angle) {
  servo.write(angle);
  delay(STEP_DELAY);
}

void moveSmooth(Servo &servo, int start, int end) {
  if(start < end) {
    for(int angle = start; angle <= end; angle += STEP_VALUE) {
      moveStep(servo, angle);
    }

  }

  else {
    for(int angle = start; angle >= end; angle -= STEP_VALUE) {
      moveStep(servo, angle);
    }
  }
}

void setup() {
  wrist.attach(9);
  elbow.attach(10);
}

void loop() {
  moveSmooth(elbow, 30, 145);
  moveSmooth(wrist, 0, 170);
  delay(500);
  moveSmooth(elbow, 145, 30);
  moveSmooth(wrist, 170, 0);
  delay(500);
}