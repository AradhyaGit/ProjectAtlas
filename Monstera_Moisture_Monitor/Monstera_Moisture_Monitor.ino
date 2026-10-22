#include <LiquidCrystal.h>

// Define your calibrated sensor limits
const int AIR_VALUE = 567;   // 0% Moisture
const int WATER_VALUE = 236; // 100% Moisture
const int soilPin = A0;      // Sensor connected to Uno Analog Pin A0

// Monstera target thresholds at 70mm depth
const int DRY_THRESHOLD = 500;     // Raw value where Monstera root zone is dry (~20%)
const int TOTAL_CYCLE_DAYS = 12;   // Average days a large plastic pot takes to dry down in Bengaluru

// Initialize Direct Parallel LCD: RS, Enable, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  Serial.begin(9600);
  
  // Initialize the LCD's size (16 columns, 2 rows)
  lcd.begin(16, 2);
  
  // Quick custom Monstera startup screen
  lcd.setCursor(0, 0);
  lcd.print("Monstera Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read raw analog value from your soilPin
  int moisture = analogRead(soilPin);
  
  // Calculate percentage using your exact calibration formula
  float moisturePercent = ((float)(AIR_VALUE - moisture) / (AIR_VALUE - WATER_VALUE)) * 100.0;
  int finalPercent = constrain((int)moisturePercent, 0, 100);
  
  // Calculate remaining days based on your 70mm depth drying curve
  int daysLeft = map(moisture, WATER_VALUE, DRY_THRESHOLD, TOTAL_CYCLE_DAYS, 0);
  daysLeft = constrain(daysLeft, 0, TOTAL_CYCLE_DAYS);
  
  // Line 1: Permanent format requested - MOISTURE: __%
  lcd.setCursor(0, 0);
  lcd.print("MOISTURE: ");
  lcd.print(finalPercent);
  lcd.print("%   "); // Extra spaces overwrite old characters
  
  // Line 2: Alternate messages using non-blocking millis()
  unsigned long currentMillis = millis();
  bool showStatusScreen = (currentMillis / 3000) % 2 == 0;
  
  lcd.setCursor(0, 1);
  if (moisture >= DRY_THRESHOLD) {
    // If deep roots hit the dry threshold, override everything and flash the alert
    if ((currentMillis / 500) % 2 == 0) {
      lcd.print("NEED WATER NOW!! ");
    } else {
      lcd.print("                "); 
    }
  } else {
    // Healthy zone: Cycle between standard status and the countdown timer
    if (showStatusScreen) {
      lcd.print("SOIL STATUS: OK ");
    } else {
      lcd.print("Water in ");
      lcd.print(daysLeft);
      if (daysLeft == 1) {
        lcd.print(" day  ");
      } else {
        lcd.print(" days ");
      }
    }
  }
  
  // Keep your original Serial printing routine active for background debugging
  Serial.println(moisture);
  
  delay(200); // 200ms refresh gives responsive blinking without slowing the CPU down
}

