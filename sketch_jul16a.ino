#include <Servo.h>
#include <lcd_i2c.h>

// Pin definitions
#define SIGNAL_PIN     D7  // Signal input from teammate's ESP8266
#define SERVO_PIN      D4  // Servo motor control
#define GREEN_LED_PIN  D2  // Green LED
#define RED_LED_PIN    D1  // Red LED
#define BUZZER_PIN     D6  // Buzzer
#define VIBRATION_PIN  D5  // Vibration sensor

// Objects
Servo servo;
lcd_i2c lcd(0x3E, 16, 2);

void setup() {
  Serial.begin(115200);

  // Inputs
  pinMode(SIGNAL_PIN, INPUT_PULLUP);
  pinMode(VIBRATION_PIN, INPUT);

  // Outputs
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Servo
  servo.attach(SERVO_PIN);
  servo.write(0);  // Locked

  // LCD
  lcd.begin();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
}

void loop() {
  // Read vibration sensor
  int vibration = digitalRead(VIBRATION_PIN);

  if (vibration == HIGH) {
    // Vibration detected
    Serial.println("Vibration detected!");

    // Activate buzzer and red LED
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);

    // Turn off green LED (override)
    digitalWrite(GREEN_LED_PIN, LOW);
  } else {
    // No vibration
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
  }

  // Only perform door logic if no vibration is happening
  if (vibration == LOW) {
    int signal = digitalRead(SIGNAL_PIN);

    if (signal == LOW) {
      // Unlock door
      servo.write(90);
      digitalWrite(GREEN_LED_PIN, HIGH);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Door opened");
    } else {
      // Lock door
      servo.write(0);
      digitalWrite(GREEN_LED_PIN, LOW);

      lcd.clear();
    }
  }

  delay(200);
}

