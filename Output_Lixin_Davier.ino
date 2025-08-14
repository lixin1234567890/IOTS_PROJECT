#include <lcd_i2c.h>
#include <Servo.h>

lcd_i2c lcd(0x3E, 16, 2);  // Your LCD address

Servo myServo;

// PIN DEFINITIONS
const int ownerPin = D7;      // Input from RFID reader (owner)
const int customerPin = D6;   // Input from customer button/sensor
const int servoPin = D4;      // Servo motor
const int ledPin = D5;        // Green LED for unlocked
const int vibrationPin = D0;  // Vibration sensor
const int buzzerPin = D8;     // Buzzer
const int redLedPin = D3;     // Red LED for vibration alert

bool unlocked = false;

void setup() {
  pinMode(ownerPin, INPUT);
  pinMode(customerPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(vibrationPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  myServo.attach(servoPin);
  myServo.write(0);  // Locked position

  lcd.begin();
  lcd.setCursor(0, 0);
  lcd.print("Scan RFID / Cust");
  lcd.setCursor(0, 1);
  lcd.print("tag or press btn");
}

void loop() {
  // --- OWNER DETECTION ---
  if (digitalRead(ownerPin) == HIGH && !unlocked) {
    myServo.write(90);         // Unlock
    digitalWrite(ledPin, LOW); // Green LED ON
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Welcome Owner ");
    unlocked = true;
  }

  // --- CUSTOMER DETECTION ---
  if (digitalRead(customerPin) == HIGH && !unlocked) {
    myServo.write(90);         // Unlock
    digitalWrite(ledPin, LOW); // Green LED ON
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Collect Parcel");
    unlocked = true;
  }

  // Reset after 4s
  if (unlocked) {
    delay(4000);
    myServo.write(0);           // Lock
    digitalWrite(ledPin, HIGH); // Green LED OFF
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan RFID / Cust");
    lcd.setCursor(0, 1);
    lcd.print("tag or press btn");
    unlocked = false;
  }

  // --- VIBRATION DETECTION ---
  if (digitalRead(vibrationPin) == HIGH) {
    digitalWrite(redLedPin, LOW);  // Red LED ON
    digitalWrite(buzzerPin, HIGH); // Buzzer ON
    delay(500);                    // Alert duration
    digitalWrite(redLedPin, HIGH); // Red LED OFF
    digitalWrite(buzzerPin, LOW);  // Buzzer OFF
  }
}

