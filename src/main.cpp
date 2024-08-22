#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// IN and OUT pins
#define moisture A0
#define potMeter A1
#define relay 2

// Values for moisture sensor calibration
#define wet 440
#define dry 828

// Soil humidity and pump time limits in percent
int minMoisture = 20;
unsigned long pumpDuration = 3000;


// Timing variables
unsigned long interval = 1000;  // Update interval for sensor readings in milliseconds
unsigned long previousMillis = 0;
unsigned long previousPumpTime = 0;
unsigned long pumpStartTime = 0;
unsigned long pumpTime = 0;
unsigned long currentPotVal = 0;
unsigned long previousPotVal = 0;

int dotCount;
bool plantHappy;
bool pumpRan = false;

// Heart character
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
// Smiley character
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

// Main loop
void loop() {
  // Read current time
  unsigned long currentMillis = millis();

  // Write to LCD
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");

  // Read moisture and convert to percent
  int soilVal = analogRead(moisture);
  int percentHumidity = map(soilVal, wet, dry, 100, 0);

  // Read potentiometer and set pumptime
  currentPotVal = analogRead(potMeter);
  long timePumped = map(currentPotVal, 0, 1023, 0, 90000);
  //Serial.println(timePumped);


  // Show current watering time on LCD
  if ((currentPotVal % previousPotVal) > 50) {
    previousPotVal = currentPotVal;

    // PRINT PUMP DURATION ON SCREEN
    lcd.setCursor(0, 1);
    lcd.print("                 ");
    lcd.setCursor(0, 1);
    lcd.print("Watering: ");
    lcd.print(timePumped/1000);
    lcd.print("s");
    delay(100);
  }
  
  // Ensure that percentage reading is between 0 and 100
  if (soilVal >= dry){
    percentHumidity = 0;
  }
  if (soilVal <= wet){
    percentHumidity = 100;
  }

  // Timed loop
  if (currentMillis - previousMillis >= interval) {
    currentPotVal = pumpDuration;

    // Variables for dot animation and timing
    dotCount = (dotCount + 1) % 4;
    previousMillis = currentMillis;

    // Display current moisture percentage
    lcd.setCursor(10, 0);
    lcd.print("      ");
    lcd.setCursor(10, 0);
    lcd.print(percentHumidity);
    lcd.print("%");
    
    // Check if watering is needed
    if (percentHumidity <= minMoisture) {
      plantHappy = true;

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
        
      // Start waterpump
      if (digitalRead(relay) == LOW and !pumpRan) {
        digitalWrite(relay, HIGH);
      }

      // Check if pump has run for the specified time
      if (currentMillis - pumpStartTime >= timePumped) {
        pumpStartTime = currentMillis;
        digitalWrite(relay, LOW);
      }
      else {
        pumpRan = true;
      }

    }
    else {
      // Update LCD when plant is happy
      if(plantHappy) {
        lcd.setCursor(0, 1);
        lcd.print("                 ");
        lcd.setCursor(0, 1);
        lcd.print("Happy plant ");
        lcd.write(1);
        //plantHappy = false;
        pumpRan = false;
        digitalWrite(relay, LOW);
        pumpStartTime = 0;
      }
    }
  }
}