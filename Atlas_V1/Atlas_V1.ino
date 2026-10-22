#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// ---------- HARDWARE ----------
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);

const byte JOY_X  = A0;
const byte JOY_Y  = A1;
const byte JOY_SW = 2;

// ---------- CONTROL TIMING ----------
const unsigned long UPDATE_MS = 20;
const unsigned long DOUBLE_CLICK_MS = 500;
const unsigned long DEBOUNCE_MS = 30;

const int DEADZONE = 65;

// Conservative initial pulse range.
// Widen only after checking each joint's mechanical limits.
const int SERVO_MIN_US = 1000;
const int SERVO_MAX_US = 2000;

// ---------- JOINT DATA ----------
struct Joint {
  const char* name;
  byte channel;
  float currentAngle;
  int minAngle;
  int maxAngle;
  float maxStepPerUpdate;
  int direction;
  int lastSentAngle;
};

// Array order is PCA9685 channel order.
// Verify the physical channel-to-joint mapping before operation.
Joint joints[6] = {
  {"gripper",      0, 90, 30, 150, 0.70f,  1, -1},
  {"wristRotate",  1, 90, 30, 150, 0.50f,  1, -1},
  {"elbow",        2, 90, 30, 150, 0.50f,  1, -1},
  {"wristPitch",   3, 90, 30, 150, 0.50f,  1, -1},
  {"base",         4, 90, 30, 150, 0.50f,  1, -1},
  {"shoulder",     5, 90, 30, 150, 0.40f,  1, -1}
};

// Gripper range is deliberately conservative until calibrated.
// Increase its limits only after verifying the linkage cannot bind.

// Control pairs, in the agreed order.
const byte GROUPS[3][2] = {
  {4, 5},
  {2, 3},
  {1, 0}
};

const char* GROUP_NAMES[3] = {
  "Channels 4 & 5",
  "Channels 2 & 3",
  "Channels 1 & 0"
};

byte activeGroup = 0;

// Starts locked; no motion until explicitly unlocked.
bool holdMode = true;

// ---------- JOYSTICK CENTER ----------
int centerX = 512;
int centerY = 512;

// ---------- BUTTON STATE ----------
bool lastRawButton = HIGH;
bool stableButton = HIGH;

unsigned long lastDebounceTime = 0;
unsigned long firstClickTime = 0;
bool waitingForSecondClick = false;

// ---------- LOOP TIMING ----------
unsigned long lastUpdateTime = 0;


// Convert a logical angle to a PCA9685 PWM pulse.
void writeJointAngle(Joint &joint, int angle) {
  angle = constrain(angle, joint.minAngle, joint.maxAngle);

  unsigned long pulseUs =
    SERVO_MIN_US +
    (unsigned long)(SERVO_MAX_US - SERVO_MIN_US) * angle / 180UL;

  unsigned long ticks = pulseUs * 4096UL / 20000UL;

  pca.setPWM(joint.channel, 0, ticks);

  joint.lastSentAngle = angle;
}


// Convert the joystick displacement into a value from -1 to +1.
float readAxisCommand(int pin, int center) {
  int raw = analogRead(pin);
  int delta = raw - center;

  if (abs(delta) <= DEADZONE) {
    return 0.0f;
  }

  float command;

  if (delta > 0) {
    command = (delta - DEADZONE) /
              (float)(1023 - center - DEADZONE);
  } else {
    command = (delta + DEADZONE) /
              (float)(center - DEADZONE);
  }

  return constrain(command, -1.0f, 1.0f);
}


// Move one joint incrementally.
void updateJoint(Joint &joint, float command) {
  if (command == 0.0f) {
    return;
  }

  joint.currentAngle +=
    command * joint.maxStepPerUpdate * joint.direction;

  joint.currentAngle = constrain(
    joint.currentAngle,
    (float)joint.minAngle,
    (float)joint.maxAngle
  );

  int roundedAngle = (int)(joint.currentAngle + 0.5f);

  if (roundedAngle != joint.lastSentAngle) {
    writeJointAngle(joint, roundedAngle);
  }
}


// Poll joystick button: single click = hold toggle,
// double-click = select the next servo pair.
void pollButton() {
  unsigned long now = millis();
  bool reading = digitalRead(JOY_SW);

  if (reading != lastRawButton) {
    lastDebounceTime = now;
    lastRawButton = reading;
  }

  if (now - lastDebounceTime >= DEBOUNCE_MS) {
    if (reading != stableButton) {
      stableButton = reading;

      if (stableButton == LOW) {
        if (waitingForSecondClick &&
            now - firstClickTime <= DOUBLE_CLICK_MS) {

          activeGroup = (activeGroup + 1) % 3;
          waitingForSecondClick = false;

          Serial.print("Selected pair: ");
          Serial.println(GROUP_NAMES[activeGroup]);

        } else {
          waitingForSecondClick = true;
          firstClickTime = now;
        }
      }
    }
  }

  if (waitingForSecondClick &&
      now - firstClickTime > DOUBLE_CLICK_MS) {

    waitingForSecondClick = false;
    holdMode = !holdMode;

    Serial.print("Motion hold: ");
    Serial.println(holdMode ? "ON" : "OFF");
  }
}


void calibrateJoystickCenter() {
  // Keep the stick untouched while powering up.
  long sumX = 0;
  long sumY = 0;

  for (int i = 0; i < 50; i++) {
    sumX += analogRead(JOY_X);
    sumY += analogRead(JOY_Y);
    delay(4);
  }

  centerX = sumX / 50;
  centerY = sumY / 50;

  Serial.print("Joystick center X: ");
  Serial.println(centerX);
  Serial.print("Joystick center Y: ");
  Serial.println(centerY);
}


void setup() {
  Serial.begin(115200);

  pinMode(JOY_SW, INPUT_PULLUP);

  pca.begin();
  pca.setPWMFreq(50);

  // Disable all PCA9685 outputs at startup.
  // Do NOT command the servos to 90 degrees automatically.
  for (byte ch = 0; ch < 16; ch++) {
    pca.setPWM(ch, 0, 4096);
  }

  calibrateJoystickCenter();

  lastRawButton = digitalRead(JOY_SW);
  stableButton = lastRawButton;

  Serial.println("Atlas controller ready.");
  Serial.println("Initially locked; double-click to change groups.");
}


void loop() {
  pollButton();

  unsigned long now = millis();

  if (now - lastUpdateTime < UPDATE_MS) {
    return;
  }

  lastUpdateTime = now;

  if (holdMode) {
    return;
  }

  float xCommand = readAxisCommand(JOY_X, centerX);
  float yCommand = readAxisCommand(JOY_Y, centerY);

  Joint &jointX = joints[GROUPS[activeGroup][0]];
  Joint &jointY = joints[GROUPS[activeGroup][1]];

  updateJoint(jointX, xCommand);
  updateJoint(jointY, yCommand);
}