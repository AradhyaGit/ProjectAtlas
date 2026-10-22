#include <Servo.h>
Servo wristServo;
Servo rotateServo;

void setup() {
  wristServo.attach(9);
  rotateServo.attach(10);
  Serial.begin(9600);
}

void loop() {
  int x = analogRead(A0);
  int y = analogRead(A1);
  int angle_x = map(x, 0, 660, 0, 180);
  int angle_y = map(y, 0, 660, 0, 180);
  wristServo.write(angle_x);
  rotateServo.write(angle_y);
  Serial.println(x);
  delay(100);
}