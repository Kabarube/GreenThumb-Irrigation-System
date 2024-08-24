/**
 * 
 * GreenThumb V1.2 - Automatic Irrigation System for Arduino
 * 
 * - Automatically waters plants with a 5v waterpump and a soil moisture sensor. 
 * - Displays moisture content, plant status and water volume on 16x2 LCD
 * - User adjustable water volume
 * 
 * @ Author: Kai Ruben Enerhaugen
 * @ Create Time: 2024-07-22
 * @ Modified by: Kai Ruben Enerhaugen
 * @ Modified time: 2024-08-24
 */


#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// Init LCD object
LiquidCrystal_I2C lcd(0x27, 16, 2);

// IN and OUT pins
#define moisture A0
#define potMeter A1
#define relay 2

// Moisture calibration and settings
#define wet 440                   // Sensor value when wet
#define dry 828                   // Sensor value when dry
#define moistureThreshold 20      // Minimum allowed humidity in %
unsigned long waterAmount = 0;    // Amount of water to be pumped

// Delays
#define measureFrequency 60000    // Frequency of moisture measurement in milliseconds (default 600000)
#define interval 200              // LCD update frequency (default 200)
#define settingsDelay 1500        // Time to display settings in milliseconds (default 1500)

// Timers
unsigned long prevDisplayTime = 0;
unsigned long prevPumpTime = 0;
unsigned long prevMenuTime = 0;
unsigned long prevMeasureTime = 0;
unsigned long currentPotVal = 0;
unsigned long previousPotVal = 0;

// Status variables
bool pumpRunning = false;
bool pumpReady = true;
bool settingsActive = false;
bool plantHappy;

// Graphics
int dotCount; // Used for heart animation

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

/** 
 * Update display
 */
void updateDisplay() {

  //TODO Implement into main loop

  unsigned long currentMillis = millis();

  // Read sensor and pot
  int soilVal = analogRead(moisture);
  int percentHumidity = map(soilVal, wet, dry, 100, 0);
  currentPotVal = int(analogRead(potMeter));

  // Set menu active if knob is turned
  if (abs(currentPotVal - previousPotVal) >= 100) {
    previousPotVal = currentPotVal;
    settingsActive = true;
    prevMenuTime = currentMillis;
  }

  // LCD refresh loop
  if (currentMillis - prevDisplayTime >= interval) {
    prevDisplayTime = currentMillis;
    
    // Display current moisture content
    lcd.setCursor(0, 0);
    lcd.print("Moisture: ");
    lcd.setCursor(10, 0);
    lcd.print("      ");
    lcd.setCursor(10, 0);
    lcd.print(percentHumidity);
    lcd.print("%");

    // Display plant health
    if (plantHappy && !settingsActive) {
      lcd.setCursor(0, 1);
      lcd.print("Happy plant ");
      lcd.write(1);
    }
    if (!plantHappy && !pumpRunning && !settingsActive) {
      lcd.setCursor(0, 1);
      lcd.print("Sad plant :(    ");
    }

    // Display heart animation when pump is running
    if (pumpRunning && !settingsActive) {
      lcd.setCursor(0, 1);
      lcd.print("Watering ");

      dotCount = (dotCount + 1) % 4;
      for (int i = 0; i < dotCount+1; i++) {
        if (i >= 3) {
          lcd.setCursor(8, 1);
          lcd.print("      ");
          lcd.setCursor(8, 1);
        }
        else {
          lcd.write(0);
          lcd.print(" ");
        }
      }
    }

    // Show settings menu
    if (settingsActive) {
      lcd.setCursor(0, 1);
      lcd.print("Water: ");
      lcd.print((waterAmount/1000)*0.18, 1);
      lcd.print(" dl ");

      // Hide settings menu after a set delay
      if (settingsActive && (currentMillis - prevMenuTime >= settingsDelay)) {
        prevMenuTime = currentMillis;
        lcd.setCursor(0, 1);
        lcd.print("                ");
        settingsActive = false;
      }
    }
  }
}

/** Timer
 * @param timer Timer iterator
 * @param time Delay in milliseconds
 * 
 * @return true, if time has passed
 */
bool timeElapsed(unsigned long &timer, unsigned long time) {
  unsigned long currentMillis = millis();

  if (currentMillis - timer >= time) {
    timer = currentMillis;
    return true;
  }
  return false;
}

// Setup
void setup() {
  // Serial init
  Serial.begin(9600);

  // Prepare pins
  pinMode(moisture, INPUT);
  pinMode(potMeter, INPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  // LCD Setup
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, Heart);
  lcd.createChar(1, Smile);
}

void loop() {
  // Read moisture sensor and pot
  int percentHumidity = map(analogRead(moisture), wet, dry, 100, 0);
  waterAmount = map(analogRead(potMeter), 0, 1023, 0, 60000);
  
  // Check if watering is needed in intervals set by 'measureFrequency'
  if (timeElapsed(prevMeasureTime, measureFrequency)) {
    Serial.println("Measuring...");

    // Start pump if moisture threshold i reached
    if (!pumpRunning && percentHumidity <= moistureThreshold) {
      digitalWrite(relay, HIGH);  // Start pump
      pumpRunning = true;         // Set pump status
      plantHappy = false;
      prevPumpTime = millis();    // Start pump timer
      Serial.println("Start Pump...");
    }
    else {
      plantHappy = true;
    }
  }

  // Stop the pump when set volume of water has been added
  if (pumpRunning && timeElapsed(prevPumpTime, waterAmount)) {
    digitalWrite(relay, LOW);   // Stop pump
    pumpRunning = false;        // Set pump status
    Serial.println("Stop Pump...");  
  }

  updateDisplay();
}