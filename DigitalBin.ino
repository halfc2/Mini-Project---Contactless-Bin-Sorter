/*
   Digital Bin - Contactless Smart Trash Sorter
   A fun, pandemic-friendly trash bin that:
   • Opens the correct lid (Plastic or Paper) when hand is detected
   • Detects when each bin is full using ultrasonic sensors
   • Sends SMS alert via GSM module when full
   • Shows cute Filipino/Gen-Z thank you messages
   • Has a manual reset button

   Made for my mentees during near after the pandemic
   Hardware: Arduino Uno/Nano + 4x HC-SR04 + SG90 Servo + 16x2 I2C LCD + GSM Module
*/

#include <Ultrasonic.h>         // Library for HC-SR04 (supports .read(CM))
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

// ========================== PIN DEFINITIONS ==========================
const uint8_t BUTTON_PIN   = A2;    // Reset button (active LOW with internal pull-up)
const uint8_t SERVO_PIN    = 3;     // Servo control (PWM)

#define TRIG1 5
#define ECHO1 6
#define TRIG2 7
#define ECHO2 8
#define TRIG3 9                     // Hand detection for PLASTIC bin
#define ECHO3 10
#define TRIG4 11                    // Hand detection for PAPER bin
#define ECHO4 12

// ========================== OBJECTS ==========================
Servo lidServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address usually 0x27 or 0x3F

Ultrasonic sensorFullPlastic(TRIG1, ECHO1);   // Inside plastic bin (full detection)
Ultrasonic sensorFullPaper(TRIG2, ECHO2);     // Inside paper bin (full detection)
Ultrasonic sensorHandPlastic(TRIG3, ECHO3);   // Hand over plastic slot
Ultrasonic sensorHandPaper(TRIG4, ECHO4);     // Hand over paper slot

// ========================== SETTINGS & THRESHOLDS ==========================
const uint8_t DISTANCE_FULL_CM     = 10;   // If object < 10 cm from top → bin considered full
const uint8_t DISTANCE_HAND_CM     = 10;   // Hand detection threshold
const uint16_t SENSOR_READ_INTERVAL = 100; // How often to poll sensors (ms)
const uint16_t LID_OPEN_TIME_MS    = 2000; // How long lid stays open after hand detected

// Servo positions (adjust these to match your physical lid mechanism)
const uint8_t SERVO_CLOSED   = 90;
const uint8_t SERVO_PLASTIC    = 10;   // Lid opens to plastic side
const uint8_t SERVO_PAPER      = 165;  // Lid opens to paper side

// ========================== STATE VARIABLES ==========================
float distFullPlastic = 0;
float distFullPaper   = 0;
float distHandPlastic = 0;
float distHandPaper   = 0;

bool plasticBinFull = false;
bool paperBinFull   = false;

bool smsSentPlastic = false;  // Prevents sending SMS multiple times
bool smsSentPaper   = false;

unsigned long lastSensorRead = 0;

// ========================== SETUP ==========================
void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lidServo.attach(SERVO_PIN);
  lidServo.write(SERVO_CLOSED);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" GROUP 9 FINAL");
  lcd.setCursor(0, 1);
  lcd.print("  Digital Bin");
  delay(2500);
  lcd.clear();
  lcd.noBacklight();
}

// ========================== MAIN LOOP ==========================
void loop() {
  unsigned long now = millis();

  // Read sensors every 100 ms
  if (now - lastSensorRead >= SENSOR_READ_INTERVAL) {
    readAllSensors();
    closeLidIfIdle();
    lastSensorRead = now;
  }

  // Full bin detection + SMS alert (only once per fill)
  if (plasticBinFull && !smsSentPlastic) {
    flashWarning("PLASTIC BIN FULL");
    sendSMS("Digital Bin: Plastic Bin is Full");
    smsSentPlastic = true;
  }
  if (paperBinFull && !smsSentPaper) {
    flashWarning("PAPER BIN FULL");
    sendSMS("Digital Bin: Paper Bin is Full");
    smsSentPaper = true;
  }

  // Keep flashing warning if hand tries to use full bin
  if (distHandPlastic < DISTANCE_HAND_CM && plasticBinFull) flashWarning("PLASTIC BIN FULL");
  if (distHandPaper   < DISTANCE_HAND_CM && paperBinFull)   flashWarning("PAPER BIN FULL");

  // Manual reset button (hold to reset everything)
  if (digitalRead(BUTTON_PIN) == LOW) {
    resetSystem();
    delay(300); // debounce
    while (digitalRead(BUTTON_PIN) == LOW); // wait for release
  }

  // Hand detected over PLASTIC slot → open lid if not full
  if (distHandPlastic < DISTANCE_HAND_CM && !plasticBinFull) {
    showMessage("PLASTIC");
    openLid(SERVO_PLASTIC);
    waitUntilHandRemoved(&distHandPlastic);
    showRandomThankYou();
  }

  // Hand detected over PAPER slot → open lid if not full
  if (distHandPaper < DISTANCE_HAND_CM && !paperBinFull) {
    showMessage("PAPER");
    openLid(SERVO_PAPER);
    waitUntilHandRemoved(&distHandPaper);
    showRandomThankYou();
  }
}

// ========================== HELPER FUNCTIONS ==========================

void readAllSensors() {
  distFullPlastic = sensorFullPlastic.read(CM);
  distFullPaper   = sensorFullPaper.read(CM);
  );
  distHandPlastic = sensorHandPlastic.read(CM);
  distHandPaper   = sensorHandPaper.read(CM  );

  // Update full status (with hysteresis: only mark full if reliably < 10 cm)
  if (distFullPlastic > 0 && distFullPlastic < DISTANCE_FULL_CM) plasticBinFull = true;
  if (distFullPaper   > 0 && distFullPaper   < DISTANCE_FULL_CM) paperBinFull   = true;
}

void closeLidIfIdle() {
  lidServo.write(SERVO_CLOSED);
}

void openLid(uint8_t position) {
  lidServo.write(position);
  delay(LID_OPEN_TIME_MS);  // Give user time to throw trash
}

void waitUntilHandRemoved(float* distance) {
  while (*distance < DISTANCE_HAND_CM && !plasticBinFull && !paperBinFull) {
    readAllSensors();  // Keep updating while waiting
    delay(50);
  }
}

void showMessage(const String& type) {
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(type);
  lcd.setCursor(0, 1);
  lcd.print("Bin Opening...");
  delay(800);
}

void flashWarning(const String& msg) {
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(msg);
  delay(2000);
  lcd.noBacklight();
}

void showRandomThankYou() {
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);

  switch (random(1, 16)) {
    case 1:  lcd.print(F("thank u :)")); break;
    case 2:  lcd.print(F("good job (uwu)b")); break;
    case 3:  lcd.print(F("arigathanks")); break;
    case 4:  lcd.print(F("slay")); break;
    case 5:  lcd.print(F("tinitigan ko")); lcd.setCursor(0,1); lcd.print(F("tinapunan ko")); break;
    case 6:  lcd.print(F("lopitt")); break;
    case 7:  lcd.print(F("galinggg")); break;
    case 8:  lcd.print(F("idol")); break;
    case 9:  lcd.print(F("salamat po!")); break;
    case 10: lcd.print(F("labyu"); break;
    case 11: lcd.print(F("pogi points +10")); break;
    case 12: lcd.print(F("ganda mo talaga")); break;
    case 13: lcd.print(F("werpa!")); break;
    case 14: lcd.print(F("nice"); break;
    case 15: lcd.print(F("salamat beh")); break;
  }
  delay(2000);
  lcd.noBacklight();
}

void sendSMS(const String& message) {
  Serial.print(F("AT+CMGF=1\r"));           // Text mode
  delay(100);
  Serial.print(F("AT+CMGS=\"+63xxxxxxxxxx\"\r"));  // Replace with your number!, +63 is for Philippine carriers
  delay(200);
  Serial.print(message);
  delay(500);
  Serial.write(0x1A);                       // Ctrl+Z
  Serial.write('\r');
  delay(2000);
}

void resetSystem() {
  plasticBinFull = paperBinFull = false;
  smsSentPlastic = smsSentPaper = false;
  lcd.clear();
  lcd.backlight();
  lcd.print("System Reset :)");
  delay(1500);
  lcd.noBacklight();
  lidServo.write(SERVO_CLOSED);
}