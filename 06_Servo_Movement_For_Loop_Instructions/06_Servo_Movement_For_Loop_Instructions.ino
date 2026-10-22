#include <Servo.h>

Servo myServo;

void setup() {
  myServo.attach(9);
}
void loop() {
  for(int angle = 0; angle <= 180; angle += 5) {
    myServo.write(angle);
    delay(20);
  }

  delay(1000);

    for(int angle = 180; angle >= 0; angle -= 3) {
      myServo.write(angle);
      delay(20);
    }

    delay(1000);
}