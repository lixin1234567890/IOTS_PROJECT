#include <Keypad.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <ArduinoJson.h>

#define transfer_pin D0

// Wi-Fi credentials
#define WIFI_SSID "..."
#define WIFI_PASSWORD "..."

// Firebase credentials
#define API_KEY "..."
#define USER_EMAIL "..."
#define USER_PASSWORD "..."
#define DATABASE_URL "..."

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// OTP Handling
bool lockoutMessageSent = false;
String currentOTP = "";
String basePath = "/deliveries/WaitingForCollection";
String userKey = "";
String otpInput = "";
bool otpMode = false;
bool lockedOut = false;
unsigned long lockoutStart = 0;
const unsigned long lockout_duration = 10000; // 10s for testing
int failedAttempts = 0;

// Keypad config
const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = {5, 4, 0, 2};      // D1, D2, D3, D4
byte colPins[COLS] = {14, 12, 13, 15};  // D5, D6, D7, D8
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(115200);

  // Ensure transfer_pin is output to avoid floating
  pinMode(transfer_pin, OUTPUT);
  digitalWrite(transfer_pin, LOW); // locked state at start (LOW)

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Assign API key and database URL
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Assign the user sign-in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Enable debug token status messages
  config.token_status_callback = tokenStatusCallback;

  // Sign in and begin Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("[System] Firebase init complete, waiting for token...");
  
  if (!Firebase.ready()) {
    Serial.print(".");
    delay(500);
  }

  if (Firebase.ready()) {
    fbdo.clear();
    if (Firebase.getJSON(fbdo, "/deliveries/WaitingForCollection")) {
      Serial.println("[System] Firebase read success");
    } else {
      Serial.println("[System] Firebase read failed: " + fbdo.errorReason());
    }
  } else {
    Serial.println("[System] Firebase not ready");
  }
  Serial.println("Press 2 to retrieve parcel.");
}

// Generate new otp if user lockout
String generateNewOtp() {
  String newOtp = String(random(100000, 999999)); 
  String otpPath = basePath + "/" + userKey + "/otp";
  
  if (Firebase.setString(fbdo, otpPath, newOtp)) {
    Serial.println("[System] New OTP generated and saved: " + newOtp);
  } else {
    Serial.println("[Error] Failed to save new OTP: " + fbdo.errorReason());
  }
  currentOTP = newOtp;
  return newOtp;
}

// Fetch otp from database
String fetchOTP(char key) {
  if (Firebase.getJSON(fbdo, basePath)) {
    String jsonStr = fbdo.stringData();
    StaticJsonDocument<1024> doc;

    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error) {
      Serial.println(error.c_str());
      return "";
    }

    if (!doc.is<JsonObject>()) {
      Serial.println("[Error] No deliveries found or invalid JSON");
      return "";
    }

    JsonObject root = doc.as<JsonObject>();
    if (root.size() == 0) {
      Serial.println("[Error] No deliveries found");
      return "";
    } 

    userKey = "";
    for (JsonPair kv : root) {
      userKey = String(kv.key().c_str()); // copy key
      break; // only first user
    }

    if (userKey == "") {
      Serial.println("[Error] No user id found under waiting for collection");
      return "";
    }

    const char* otp = root[userKey]["otp"];
    if (otp == nullptr) {
      Serial.println("[Error] OTP field missing for user: " + userKey);
      return "";
    }

    currentOTP = String(otp);
    Serial.println("[System] OTP fetched: " + currentOTP);
  } else {
    Serial.println("[Error] Cannot access firebase path: " + fbdo.errorReason());
  }

  // Trigger sending email if '2' was pressed
  if (key == '2') {
    if (userKey != "") {
      if (Firebase.setString(fbdo, "/sendOtpTrigger/userId", userKey)) {
        Serial.println("📧 OTP Email will send shortly");
      } else {
        Serial.println("[System] Failed to send trigger: " + fbdo.errorReason());
      }
    } else {
      Serial.println("[System] UserKey is empty, cannot send trigger");
    }
  }
  return currentOTP;
}

void moveUserToCollected(const String& userId) {
  String sourcePath = "/deliveries/WaitingForCollection/" + userId;
  String destPath = "/deliveries/Collected/" + userId;

  if (!Firebase.getJSON(fbdo, sourcePath)) {
    Serial.println("[System] Failed to get user data: " + fbdo.errorReason());
  }

  String rawJson = fbdo.jsonString();
  FirebaseJson fj;
  if (fj.setJsonData(rawJson)) {
    if (!Firebase.set(fbdo, destPath, fj)) {
      Serial.println("[System] Failed to set user data at destination: " + fbdo.errorReason());
    }
    if (!Firebase.deleteNode(fbdo, sourcePath)) {
      Serial.println("[System] Failed to remove user data at source: " + fbdo.errorReason());
    }
  } else {
    Serial.println("[System] Failed to parse JSON into FirebaseJson");
  }
}

// Unlock function with guaranteed reset
void unlockAndRelock() {
  digitalWrite(transfer_pin, HIGH); // unlock
  delay(2000);
  digitalWrite(transfer_pin, LOW);  // lock again
}

// Handle keypad inputs
void handleOTP(char key) {
  if (!otpMode || key == NO_KEY) return;

  if (lockedOut) {
    if (millis() - lockoutStart >= lockout_duration) {
      lockedOut = false;
      failedAttempts = 0;
      otpMode = false;
      otpInput = "";
      Serial.println("[System] Lockout ended.");
      generateNewOtp();
      Serial.println("New OTP has been sent");
    } else {
      Serial.println("[System] Still locked.");
    }
    return;
  }

  if (key >= '0' && key <= '9') {
    otpInput += key;
    Serial.println("[Input] " + otpInput);
  } else if (key == '*') {
    otpInput = "";
    Serial.println("Input cleared, please enter again:");
  } else if (key == '#') {
    if (otpInput.length() != 6) {
      Serial.println("OTP must be 6 digits, please try again.");
      return;
    }

    if (otpInput == currentOTP) {
      Serial.println("Correct OTP! Unlocking...");
      unlockAndRelock();
      moveUserToCollected(userKey);
      otpMode = false;
      failedAttempts = 0;
    } else {
      failedAttempts++;
      Serial.println("Incorrect OTP. Please try again.");
      if (failedAttempts >= 3) {
        lockedOut = true;
        lockoutStart = millis();
        Serial.println("Too many attempts, try again in one minute");
      }
    }
    otpInput = "";
  }
}

void loop() {
  char key = keypad.getKey();

  if (key == '2' && !otpMode) {
    Serial.println("Pressed 2 - Starting OTP Retrieval");
    otpMode = true;
    otpInput = "";
    String otp = fetchOTP(key);
    if (otp == "") {
      Serial.println("[Error] Failed to get otp, reset and try again.");
      otpMode = false;
    }
    Serial.println("Enter your otp:"); 
    return;
  }
  handleOTP(key);
  delay(200);
}
