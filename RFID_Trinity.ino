#include <SPI.h>
#include <MFRC522.h>

// RFID Pins
#define SS_PIN  D8  // The ESP8266 pin D8
#define RST_PIN D2  // The ESP8266 pin D2

// Output to 3rd ESP8266 (e.g., for servo or LED)
#define TRANSFER_PIN D4

MFRC522 mfrc522(SS_PIN, RST_PIN);  // RFID reader object
bool cardPresent = false;          // Track if a card is currently detected

void setup() {
  Serial.begin(115200);

  // Initialize RFID
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID Reader initialized");

  // Setup output pin to signal 3rd ESP8266
  pinMode(TRANSFER_PIN, OUTPUT);
  digitalWrite(TRANSFER_PIN, LOW); // Default LOW
}

void loop() {
  // Check if a new card is detected and readable
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    if (!cardPresent) {
      cardPresent = true;  // Mark card as present

      // Print UID for debugging
      Serial.print("UID tag: ");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
      }
      Serial.println();
      Serial.println("Card detected successfully");

      // Send HIGH pulse to 3rd ESP8266
      digitalWrite(TRANSFER_PIN, HIGH);
      Serial.println("Sending signal to open door");
      delay(500);  // Duration of HIGH pulse
      digitalWrite(TRANSFER_PIN, LOW);

      // Halt RFID communication
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
  } else {
    // If no card is currently detected, allow next detection
    cardPresent = false;
  }

  delay(50);  // Small delay to avoid excessive polling
}

