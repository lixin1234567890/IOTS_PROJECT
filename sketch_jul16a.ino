#include <Servo.h>
#include <lcd_i2c.h>

// Pin definitions
#define ACCESS_SIGNAL    D7    // Combined Owner/User input
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
  pinMode(ACCESS_SIGNAL, INPUT);
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
  bool accessGranted = digitalRead(ACCESS_SIGNAL) == HIGH;
  bool vibrationDetected = digitalRead(VIBRATION_PIN) == HIGH;

  // Vibration response
  if (vibrationDetected) {
    digitalWrite(RED_LED, LOW);   // Turn on red LED (active-low)
  } else {
    digitalWrite(RED_LED, HIGH);  // Turn off red LED
  }

  // Access granted
  if (accessGranted) {
    servo.write(90);               // Unlock
    digitalWrite(GREEN_LED, LOW); // Turn on green LED (active-low)

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome");
    Serial.println("HIGH");
  } else {
    // Default state
    servo.write(0);                // Lock
    digitalWrite(GREEN_LED, HIGH); // Turn off green LED
    lcd.clear();
    Serial.println("LOW");
  }
  delay(300);
}

