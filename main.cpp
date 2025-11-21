#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <ESP32Servo.h>

// ============================================================
// 1. CONFIG & CONSTANTS (Magic Numbers Yok Edildi)
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

// Zamanlama Ayarları (Milisaniye cinsinden)
#define TIME_DOOR_OPEN  3000UL  // Kapı 3 saniye açık kalsın (UL: Unsigned Long)
#define TIME_ERROR_SHOW 2000UL  // Hata mesajı 2 saniye dursun
#define BLINK_INTERVAL  200UL   // Kırmızı LED 200ms aralıkla yanıp sönsün

// Şifre
const String MASTER_PASS = "1234";
const int PASS_LEN = 4;

// ============================================================
// 2. NESNELER VE DEĞİŞKENLER
// ============================================================

// Donanım Nesneleri
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo kapiServosu;

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
  STATE_LOCKED,     // Bekleme modu
  STATE_OPEN,       // Kapı açık
  STATE_ERROR       // Yanlış şifre modu
};

SystemState currentState = STATE_LOCKED;

// Global Değişkenler
String inputBuffer = "";
unsigned long stateStartTime = 0;    // Durumun başladığı an
unsigned long lastBlinkTime = 0;     // LED yanıp sönme zamanlayıcısı
bool ledState = false;               // Yanıp sönme durumu için

// ============================================================
// 3. FONKSİYON PROTOTİPLERİ (Forward Declaration)
// ============================================================
void checkKeypad();
void processPassword();
void unlockDoor();
void lockDoor();
void triggerError();
void updateSystemState(); // Zamanlayıcıları ve durum geçişlerini yönetir
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

  // Sistemi kilitli başlat
  lockDoor();
}

// ============================================================
// 5. LOOP (Tertemiz!)
// ============================================================
void loop() {
  // 1. Tuş takımını dinle
  checkKeypad();

  // 2. Sistemin o anki durumuna göre zamanlayıcıları yönet
  updateSystemState();
}

// ============================================================
// 6. FONKSİYON DETAYLARI
// ============================================================

void checkKeypad() {
  // Eğer sistem şu an meşgulse (Kapı açıksa veya Hata veriyorsa) tuş okuma
  if (currentState != STATE_LOCKED) return;

  char key = keypad.getKey();

  if (key) {
    Serial.println(key); // Debug için

    // Şifre tamponuna ekle
    inputBuffer += key;

    // Ekrana yıldız bas
    lcd.setCursor(inputBuffer.length(), 1); // "LOCKED" yazısının altına değil, yanına
    lcd.print("*");

    // Şifre tamamlandı mı?
    if (inputBuffer.length() == PASS_LEN) {
      processPassword();
    }
  }
}

void processPassword() {
  if (inputBuffer == MASTER_PASS) {
    unlockDoor();
  } else {
    triggerError();
  }
  // Tamponu temizle
  inputBuffer = "";
}

void unlockDoor() {
  currentState = STATE_OPEN;
  stateStartTime = millis(); // Kronometreyi başlat

  kapiServosu.write(ANGLE_OPEN);
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_RED, LOW);
  
  displayMessage("ACCESS GRANTED", "    OPEN");
}

void triggerError() {
  currentState = STATE_ERROR;
  stateStartTime = millis(); // Hata süresi kronometresi
  lastBlinkTime = millis();  // Blink kronometresi

  displayMessage("ACCESS DENIED", "  WRONG PASS");
}

void lockDoor() {
  currentState = STATE_LOCKED;
  
  kapiServosu.write(ANGLE_LOCKED);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, LOW);
  
  displayMessage("SYSTEM LOCKED", "Pass: ");
  
  inputBuffer = ""; // Güvenlik için buffer'ı temizle
}

// Bu fonksiyon loop içinde sürekli döner ve süresi dolan işlemleri bitirir
void updateSystemState() {
  unsigned long currentMillis = millis();

  switch (currentState) {
    
    case STATE_OPEN:
      // Kapı açık kalma süresi doldu mu?
      if (currentMillis - stateStartTime >= TIME_DOOR_OPEN) {
        lockDoor();
      }
      break;

    case STATE_ERROR:
      // 1. LED Yanıp Sönme (Blink Without Delay)
      if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
        lastBlinkTime = currentMillis; // Zamanı güncelle
        ledState = !ledState;          // Durumu tersine çevir
        digitalWrite(PIN_LED_RED, ledState);
      }

      // 2. Hata gösterme süresi doldu mu?
      if (currentMillis - stateStartTime >= TIME_ERROR_SHOW) {
        lockDoor();
      }
      break;

    case STATE_LOCKED:
      // Kilitliyken özel bir zamanlayıcıya ihtiyacımız yok
      break;
  }
}

// LCD Mesajlarını tek merkezden yönetmek için yardımcı fonksiyon
void displayMessage(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}
