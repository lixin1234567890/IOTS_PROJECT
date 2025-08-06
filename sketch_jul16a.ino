#include <Servo.h>
#include <lcd_i2c.h>

// Pin definitions
#define OWNER_SIGNAL     D7    // Owner input
#define USER_SIGNAL      D3    // User input
#define SERVO_PIN        D4    // Servo control
#define GREEN_LED        D2    // Green LED (door open)
#define VIBRATION_PIN    D5    // Vibration sensor
#define RED_LED          D6    // Red LED (vibration alert)

// Objects
Servo servo;
lcd_i2c lcd(0x3E, 16, 2); // LCD address, cols, rows

void setup() {
  Serial.begin(115200);

  // Inputs
  pinMode(OWNER_SIGNAL, INPUT);
  pinMode(USER_SIGNAL, INPUT);
  pinMode(VIBRATION_PIN, INPUT);

  // Outputs
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // Servo setup
  servo.attach(SERVO_PIN);
  servo.write(0); // Locked

  // LCD setup
  lcd.begin();
  lcd.clear();
}

void loop() {
  bool ownerActive = digitalRead(OWNER_SIGNAL) == HIGH;
  bool userActive = digitalRead(USER_SIGNAL) == HIGH;
  bool vibrationDetected = digitalRead(VIBRATION_PIN) == HIGH;

  // Vibration response
  if (vibrationDetected) {
    digitalWrite(RED_LED, HIGH);
  } else {
    digitalWrite(RED_LED, LOW);
  }

  // Owner or user access
  if (ownerActive || userActive) {
    servo.write(90);  // Unlock
    digitalWrite(GREEN_LED, HIGH);

    lcd.clear();
    lcd.setCursor(0, 0);
    if (ownerActive) {
      lcd.print("Welcome, owner");
    } else {
      lcd.print("Welcome, user");
    }
  } else {
    // Default state
    servo.write(0);  // Lock
    digitalWrite(GREEN_LED, LOW);
    lcd.clear();
  }

  delay(300);  // Adjust for responsiveness
}


