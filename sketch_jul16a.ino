#include <Servo.h>
#include <lcd_i2c.h>

// Pins
#define OWNER_SIGNAL D7     // Input signal for Owner
#define USER_SIGNAL  D3     // Input signal for User
#define SERVO_PIN    D4     // Servo motor control
#define GREEN_LED    D2     // Green LED output

// Objects
Servo servo;
lcd_i2c lcd(0x3E, 16, 2); // I2C LCD: address, cols, rows

void setup() {
  Serial.begin(115200);

  // Input setup (Active HIGH)
  pinMode(OWNER_SIGNAL, INPUT);
  pinMode(USER_SIGNAL, INPUT);

  // Output setup
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);  // LED off by default

  // Servo setup
  servo.attach(SERVO_PIN);
  servo.write(0);  // Locked by default

  // LCD setup
  lcd.begin();
  lcd.clear();
}

void loop() {
  bool ownerActive = digitalRead(OWNER_SIGNAL) == HIGH;
  bool userActive  = digitalRead(USER_SIGNAL) == HIGH;

  if (ownerActive || userActive) {
    // Unlock door
    servo.write(90);  // Adjust angle if needed
    digitalWrite(GREEN_LED, HIGH);

    // Show welcome message
    lcd.clear();
    lcd.setCursor(0, 0);
    if (ownerActive) {
      lcd.print("Welcome, owner");
    } else {
      lcd.print("Welcome, user");
    }
  } else {
    // No signal: reset system
    servo.write(0);  // Lock door
    digitalWrite(GREEN_LED, LOW);
    lcd.clear();
  }

  delay(300);  // Adjust as needed
}


