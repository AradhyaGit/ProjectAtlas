#include <LiquidCrystal.h>

// ============================================================
// SENSOR CALIBRATION
// ============================================================

const int AIR_VALUE = 567;
const int WATER_VALUE = 236;

const int soilPin = A0;
const int buttonPin = 7;


// ============================================================
// PLANT PROFILES
// ============================================================

const int TOTAL_PLANTS = 3;
int currentPlant = 0;

struct PlantProfile {
  String name;
  int dryThreshold;
  int cycleDays;
  int iconLeftIndex;
  int iconRightIndex;
};

PlantProfile plants[TOTAL_PLANTS] = {
  // Name          Dry threshold   Cycle estimate   Icon
  {"Monstera",     500,            12,               0, 1},
  {"Anthurium",    450,            7,                2, 3},
  {"Periwinkle",   510,            10,               4, 5}
};


// ============================================================
// LCD
// ============================================================

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


// ============================================================
// CUSTOM CHARACTERS
// ============================================================

// ------------------------------------------------------------
// MONSTERA - LEFT
// ------------------------------------------------------------

byte monsteraLeft[8] = {
  B00011,
  B00111,
  B01110,
  B11101,
  B11111,
  B01111,
  B00111,
  B00011
};


// ------------------------------------------------------------
// MONSTERA - RIGHT
// ------------------------------------------------------------

byte monsteraRight[8] = {
  B11000,
  B11100,
  B01110,
  B10111,
  B11111,
  B11110,
  B11100,
  B11000
};


// ------------------------------------------------------------
// ANTHURIUM - LEFT
// ------------------------------------------------------------

byte anthuriumLeft[8] = {
  B00111,
  B01111,
  B11111,
  B11110,
  B11100,
  B01100,
  B00100,
  B00000
};


// ------------------------------------------------------------
// ANTHURIUM - RIGHT
// ------------------------------------------------------------

byte anthuriumRight[8] = {
  B11100,
  B11110,
  B11101,
  B01111,
  B00111,
  B00110,
  B00100,
  B00000
};


// ------------------------------------------------------------
// PERIWINKLE - LEFT
// ------------------------------------------------------------

byte periwinkleLeft[8] = {
  B00000,
  B00110,
  B01111,
  B11111,
  B11111,
  B01111,
  B00110,
  B00000
};


// ------------------------------------------------------------
// PERIWINKLE - RIGHT
// ------------------------------------------------------------

byte periwinkleRight[8] = {
  B00000,
  B01100,
  B11110,
  B11111,
  B11111,
  B11110,
  B01100,
  B00000
};


// ============================================================
// BUTTON
// ============================================================

bool lastButtonState = HIGH;
unsigned long buttonPressedTime = 0;

const unsigned long longPressDuration = 1500;


// ============================================================
// DISPLAY SETTINGS
// ============================================================

const unsigned long screenDuration = 5000;


// ============================================================
// CENTER TEXT
// ============================================================

void printCentered(String text, int row) {

  int startPosition = (16 - text.length()) / 2;

  if (startPosition < 0) {
    startPosition = 0;
  }

  lcd.setCursor(0, row);
  lcd.print("                ");

  lcd.setCursor(startPosition, row);
  lcd.print(text);
}


// ============================================================
// DISPLAY PLANT ICON
// ============================================================

void displayPlantIcon(int plantIndex) {

  lcd.setCursor(7, 1);

  lcd.write(byte(plants[plantIndex].iconLeftIndex));
  lcd.write(byte(plants[plantIndex].iconRightIndex));
}


// ============================================================
// DISPLAY PLANT SELECTION
// ============================================================

void showPlantSelection() {

  lcd.clear();

  printCentered(plants[currentPlant].name, 0);

  displayPlantIcon(currentPlant);
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(9600);

  pinMode(buttonPin, INPUT_PULLUP);

  lcd.begin(16, 2);


  // ==========================================================
  // LOAD CUSTOM CHARACTERS
  // ==========================================================

  lcd.createChar(0, monsteraLeft);
  lcd.createChar(1, monsteraRight);

  lcd.createChar(2, anthuriumLeft);
  lcd.createChar(3, anthuriumRight);

  lcd.createChar(4, periwinkleLeft);
  lcd.createChar(5, periwinkleRight);


  // ==========================================================
  // STARTUP SCREEN
  // ==========================================================

  lcd.clear();

  printCentered("PLANT MONITOR", 0);

  lcd.setCursor(0, 1);

  for (int i = 0; i < 16; i++) {

    lcd.write(byte(255));

    delay(375);
  }


  // ==========================================================
  // SCREEN WIPE
  // ==========================================================

  for (int i = 0; i < 16; i++) {

    lcd.setCursor(i, 0);
    lcd.print(" ");

    lcd.setCursor(i, 1);
    lcd.print(" ");

    delay(62);
  }

  lcd.clear();


  // ==========================================================
  // CHOOSE A PLANT
  // ==========================================================

  lcd.setCursor(0, 0);
  lcd.print("CHOOSE A PLANT:");

  lcd.setCursor(0, 1);
  lcd.print("Press To Start");


  // Wait for first button press
  while (digitalRead(buttonPin) == HIGH) {
    delay(10);
  }


  // Wait for release
  while (digitalRead(buttonPin) == LOW) {
    delay(10);
  }

  delay(250);


  // ==========================================================
  // PLANT SELECTION
  // ==========================================================

  bool choosing = true;

  lastButtonState = HIGH;
  buttonPressedTime = 0;

  showPlantSelection();


  while (choosing) {

    bool currentButtonState = digitalRead(buttonPin);


    // Button pressed
    if (currentButtonState == LOW &&
        lastButtonState == HIGH) {

      buttonPressedTime = millis();
    }


    // Long press = confirm
    if (currentButtonState == LOW &&
        buttonPressedTime > 0) {

      if ((millis() - buttonPressedTime)
          >= longPressDuration) {

        choosing = false;

        buttonPressedTime = 0;

        delay(200);
      }
    }


    // Button released
    if (currentButtonState == HIGH &&
        lastButtonState == LOW &&
        buttonPressedTime > 0) {

      unsigned long pressDuration =
        millis() - buttonPressedTime;

      buttonPressedTime = 0;


      // Short press = next plant
      if (pressDuration < longPressDuration) {

        currentPlant++;

        if (currentPlant >= TOTAL_PLANTS) {
          currentPlant = 0;
        }

        showPlantSelection();
      }
    }


    lastButtonState = currentButtonState;

    delay(20);
  }


  // ==========================================================
  // PLANT CONFIRMED
  // ==========================================================

  lcd.clear();

  printCentered(plants[currentPlant].name, 0);
  printCentered("Selected!", 1);

  delay(1200);


  // ==========================================================
  // VITALS LOADING
  // ==========================================================

  lcd.clear();

  printCentered(plants[currentPlant].name, 0);


  for (int loopCount = 0; loopCount < 3; loopCount++) {

    for (int dots = 0; dots <= 3; dots++) {

      lcd.setCursor(0, 1);
      lcd.print("                ");

      lcd.setCursor(4, 1);
      lcd.print("Checking");

      if (dots >= 1) {
        lcd.print(".");
      }

      if (dots >= 2) {
        lcd.print(".");
      }

      if (dots >= 3) {
        lcd.print(".");
      }

      delay(500);
    }
  }

  lcd.clear();
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop() {

  // ==========================================================
  // BUTTON
  // ==========================================================

  bool currentButtonState = digitalRead(buttonPin);


  // Button pressed
  if (currentButtonState == LOW &&
      lastButtonState == HIGH) {

    buttonPressedTime = millis();
  }


  // Button released
  if (currentButtonState == HIGH &&
      lastButtonState == LOW &&
      buttonPressedTime > 0) {

    unsigned long pressDuration =
      millis() - buttonPressedTime;

    buttonPressedTime = 0;


    // Short press = change plant
    if (pressDuration < longPressDuration) {

      currentPlant++;

      if (currentPlant >= TOTAL_PLANTS) {
        currentPlant = 0;
      }


      lcd.clear();

      printCentered("PROFILE", 0);
      printCentered(plants[currentPlant].name, 1);

      delay(1500);

      lcd.clear();
    }
  }


  lastButtonState = currentButtonState;


  // ==========================================================
  // SOIL SENSOR
  // ==========================================================

  int moisture = analogRead(soilPin);


  float moisturePercent =
    ((float)(AIR_VALUE - moisture) /
     (AIR_VALUE - WATER_VALUE)) * 100.0;


  int finalPercent =
    constrain((int)moisturePercent, 0, 100);


  // ==========================================================
  // WATERING ESTIMATE
  // ==========================================================

  int daysLeft =
    map(
      moisture,
      WATER_VALUE,
      plants[currentPlant].dryThreshold,
      plants[currentPlant].cycleDays,
      0
    );


  daysLeft =
    constrain(
      daysLeft,
      0,
      plants[currentPlant].cycleDays
    );


  // ==========================================================
  // DISPLAY
  // ==========================================================

  unsigned long currentMillis = millis();


  // Always show plant name
  printCentered(
    plants[currentPlant].name,
    0
  );


  // ==========================================================
  // DRY SOIL WARNING
  // ==========================================================

  if (moisture >=
      plants[currentPlant].dryThreshold) {

    if ((currentMillis / 700) % 2 == 0) {

      printCentered("WATER NOW!", 1);

    } else {

      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
  }


  // ==========================================================
  // NORMAL OPERATION
  // ==========================================================

  else {

    int screen =
      (currentMillis / screenDuration) % 3;


    // --------------------------------------------------------
    // MOISTURE
    // --------------------------------------------------------

    if (screen == 0) {

      String moistureText =
        "Moisture: " +
        String(finalPercent) +
        "%";

      printCentered(moistureText, 1);
    }


    // --------------------------------------------------------
    // SOIL STATUS
    // --------------------------------------------------------

    else if (screen == 1) {

      printCentered("Soil Status: OK", 1);
    }


    // --------------------------------------------------------
    // WATERING ESTIMATE
    // --------------------------------------------------------

    else {

      String waterText =
        "Water in " +
        String(daysLeft);

      if (daysLeft == 1) {
        waterText += " day";
      }

      else {
        waterText += " days";
      }

      printCentered(waterText, 1);
    }
  }


  // ==========================================================
  // SERIAL MONITOR
  // ==========================================================

  Serial.print("Monitoring: ");
  Serial.print(plants[currentPlant].name);

  Serial.print(" | Raw Pin A0: ");
  Serial.print(moisture);

  Serial.print(" | Calibrated: ");
  Serial.print(finalPercent);

  Serial.println("%");


  delay(100);
}