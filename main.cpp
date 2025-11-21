#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include <Preferences.h> // <--- YENİ: Kalıcı hafıza kütüphanesi

// ============================================================
// 1. CONFIG & CONSTANTS
// ============================================================

// Pin Tanımları
#define PIN_SERVO       26
#define PIN_LED_GREEN   32
#define PIN_LED_RED     33

// Keypad Pinleri
const byte PIN_ROW_1 = 19;
const byte PIN_ROW_2 = 18;
const byte PIN_ROW_3 = 5;
const byte PIN_ROW_4 = 17;
const byte PIN_COL_1 = 16;
const byte PIN_COL_2 = 4;
const byte PIN_COL_3 = 0;
const byte PIN_COL_4 = 2;

// Servo Ayarları
#define SERVO_MIN_US    500
#define SERVO_MAX_US    2400
#define SERVO_FREQ      50
#define ANGLE_LOCKED    0
#define ANGLE_OPEN      90

// Zamanlama Ayarları
#define TIME_DOOR_OPEN  3000UL
#define TIME_ERROR_SHOW 2000UL
#define BLINK_INTERVAL  200UL

// Şifre Ayarları
const int PASS_LEN = 4;

// ============================================================
// 2. NESNELER VE DEĞİŞKENLER
// ============================================================

// Donanım Nesneleri
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo kapiServosu;
Preferences preferences; // <--- YENİ: Hafıza nesnesi

// Keypad Ayarları
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {PIN_ROW_1, PIN_ROW_2, PIN_ROW_3, PIN_ROW_4};
byte colPins[COLS] = {PIN_COL_1, PIN_COL_2, PIN_COL_3, PIN_COL_4};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Durum Yönetimi (State Machine)
enum SystemState {
  STATE_LOCKED,
  STATE_OPEN,
  STATE_ERROR
};

SystemState currentState = STATE_LOCKED;

// Global Değişkenler
String inputBuffer = "";
String currentPassword;      // <--- YENİ: Değiştirilebilir şifre değişkeni
unsigned long stateStartTime = 0;
unsigned long lastBlinkTime = 0;
bool ledState = false;

// ============================================================
// 3. FONKSİYON PROTOTİPLERİ
// ============================================================
void checkKeypad();
void processPassword();
void unlockDoor();
void lockDoor();
void triggerError();
void updateSystemState();
void displayMessage(String line1, String line2);

// ============================================================
// 4. SETUP
// ============================================================
void setup() {
  Serial.begin(115200);

  // Pin Ayarları
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);

  // Servo Başlatma
  kapiServosu.setPeriodHertz(SERVO_FREQ);
  kapiServosu.attach(PIN_SERVO, SERVO_MIN_US, SERVO_MAX_US);
  
  // LCD Başlatma
  lcd.init();
  lcd.backlight();

  // --- YENİ: HAFIZA İŞLEMLERİ ---
  // "safe-app" namespace'ini açıyoruz. false = okuma/yazma modu.
  preferences.begin("safe-app", false); 
  
  // Hafızada "pass" anahtarı var mı kontrol et, yoksa "" döner.
  String savedPass = preferences.getString("pass", ""); 
  
  if (savedPass == "") {
    Serial.println("Hafizada sifre yok, varsayilan ataniyor...");
    currentPassword = "1234";             // Varsayılan fabrika şifresi
    preferences.putString("pass", currentPassword); // Hafızaya kaydet
  } else {
    Serial.println("Kayitli sifre hafizadan yuklendi.");
    currentPassword = savedPass;          // Hafızadaki şifreyi al
  }
  
  Serial.println("Gecerli Sifre: " + currentPassword); // Debug için (Güvenlikte kaldırılır)

  // Sistemi kilitli başlat
  lockDoor();
}

// ============================================================
// 5. LOOP
// ============================================================
void loop() {
  checkKeypad();
  updateSystemState();
}

// ============================================================
// 6. FONKSİYON DETAYLARI
// ============================================================

void checkKeypad() {
  if (currentState != STATE_LOCKED) return;

  char key = keypad.getKey();

  if (key) {
    // Şifre tamponuna ekle
    inputBuffer += key;

    // Ekrana yıldız bas
    lcd.setCursor(inputBuffer.length(), 1); 
    lcd.print("*");

    // Şifre uzunluğu tamamlandı mı?
    if (inputBuffer.length() == PASS_LEN) {
      processPassword();
    }
  }
}

void processPassword() {
  // <--- GÜNCELLEME: Artık sabit değil, değişkene bakıyor
  if (inputBuffer == currentPassword) {
    unlockDoor();
  } else {
    triggerError();
  }
  inputBuffer = "";
}

void unlockDoor() {
  currentState = STATE_OPEN;
  stateStartTime = millis();

  kapiServosu.write(ANGLE_OPEN);
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_RED, LOW);
  
  displayMessage("ACCESS GRANTED", "    OPEN");
}

void triggerError() {
  currentState = STATE_ERROR;
  stateStartTime = millis();
  lastBlinkTime = millis();

  displayMessage("ACCESS DENIED", "  WRONG PASS");
}

void lockDoor() {
  currentState = STATE_LOCKED;
  
  kapiServosu.write(ANGLE_LOCKED);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, LOW);
  
  displayMessage("SYSTEM LOCKED", "Pass: ");
  
  inputBuffer = "";
}

void updateSystemState() {
  unsigned long currentMillis = millis();

  switch (currentState) {
    case STATE_OPEN:
      if (currentMillis - stateStartTime >= TIME_DOOR_OPEN) {
        lockDoor();
      }
      break;

    case STATE_ERROR:
      if (currentMillis - stateStartTime >= TIME_ERROR_SHOW) {
        lockDoor();
      }
      // LED Yanıp Sönme (Non-blocking)
      if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
        lastBlinkTime = currentMillis;
        ledState = !ledState;
        digitalWrite(PIN_LED_RED, ledState);
      }
      break;

    case STATE_LOCKED:
      break;
  }
}

void displayMessage(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}
