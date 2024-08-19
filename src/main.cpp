#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// IN and OUT pins
int moisture = A0;
int relay = 3;
bool plantHappy;

// Values for moisture sensor calibration
int wet = 440;  // Value for wet sensor
int dry = 828;  // Value for dry sensor

// Soil humidity limits in percent
int minMoisture = 20;

// Timing variables
const int interval = 500;  // Update interval for sensor readings in milliseconds
unsigned long previousMillis = 0;
int dotCount;

// Custom characters
byte Heart[] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

byte Smile[] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b10001,
  0b01110,
  0b00000,
  0b00000
  };

// Setup
void setup() {
  // Set pin modes
  pinMode(moisture, INPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  // LCD Setup
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, Heart);
  lcd.createChar(1, Smile);
}

// Main loop
void loop() {
  unsigned long currentMillis = millis(); // Read current time
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");  // Print to screen

  // Update only the numbers on the LCD using millis
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    int soilVal = analogRead(moisture); // Read moisture
    int percentHumidity = map(soilVal, wet, dry, 100, 0); // Convert to percent
    
    // Ensure that percentage reading is between 0 and 100
    if (soilVal >= dry){
      percentHumidity = 0;
    }
    if (soilVal <= wet){
      percentHumidity = 100;
    }

    // Display current moisture percentage
    lcd.setCursor(10, 0);
    lcd.print("      ");
    lcd.setCursor(10, 0);
    lcd.print(percentHumidity);
    lcd.print(" %");

    // Counter for dot animation
    dotCount = (dotCount + 1) % 4;

    // Check if watering is needed
    if (percentHumidity <= minMoisture) {
      plantHappy = false; // Set plant status
      digitalWrite(relay, HIGH);  // Turn on waterpump

      // Display watering status
      lcd.setCursor(0, 1);
      lcd.print("Watering ");

      // Dot animation
      for (int i = 0; i < dotCount+1; i++) {
        if (i >= 3) {
          lcd.setCursor(8, 1);
          lcd.print("      ");
        }
        else {
          lcd.write(0);
          lcd.print(" ");
        }
      }
    }
    else {
      // Turn off water pump
      digitalWrite(relay, LOW);

      // Update LCD when plant is happy
      if(!plantHappy) {
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.print("Happy plant ");
        lcd.write(1);
        plantHappy = true;
      }
    }
  }
}

