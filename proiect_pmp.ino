#include <DHT.h>

// --- Soil moisture sensor ---
const int soilPin = A0;      // soil moisture sensor (AOUT)

// --- Pump relay ---
const int relayPin = 7;      // relay control pin
int DRY_THRESHOLD = 400;     // soil dry threshold
const bool RELAY_ACTIVE_LOW = false;

// --- DHT11 sensor ---
#define DHTPIN 8
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- Water level sensor + buzzer ---
const int waterLevelPin = A1;  // water level sensor (analog)
const int buzzerPin     = 10;  // buzzer on D10
const int WATER_LOW_THRESHOLD = 300;  // below this = water too low

// --- Light sensor (HW-072 DO) + Light bulb ---
const int lightDigitalPin = 11; // HW-072 DO
const int ledPin = 12;          // bec / LED (ON when DARK)

int waterLevelValue = 0;

void setPump(bool on) {
  if (RELAY_ACTIVE_LOW)
    digitalWrite(relayPin, on ? LOW : HIGH);  // LOW = ON
  else
    digitalWrite(relayPin, on ? HIGH : LOW);  // HIGH = ON
}

void setup() {
  Serial.begin(9600);

  pinMode(relayPin, OUTPUT);
  setPump(false);

  pinMode(buzzerPin, OUTPUT);
  noTone(buzzerPin);

  // --- Light sensor + LED ---
  pinMode(lightDigitalPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  dht.begin();

  Serial.println("System started (pump + soil + DHT11 + water + buzzer + light)...");
}

void loop() {
  // --- Read soil moisture ---
  int soilValue = analogRead(soilPin);

  // --- Read water level ---
  waterLevelValue = analogRead(waterLevelPin);

  // --- Read light sensor ---
  int lightVal = digitalRead(lightDigitalPin);
  bool isDark = (lightVal == HIGH);   // LOGICA TA: HIGH = DARK

  // --- Control light bulb ---
  digitalWrite(ledPin, isDark ? HIGH : LOW);

  // -----------------------------
  // BUZZER LOGIC
  // -----------------------------
  bool waterLow = (waterLevelValue < WATER_LOW_THRESHOLD);

  if (waterLow) {
    tone(buzzerPin, 1000);     // 1000 Hz alarm tone
  } else {
    noTone(buzzerPin);
  }

  // -----------------------------
  // PUMP LOGIC (nemodificat)
  // -----------------------------
  bool soilDry = (soilValue < DRY_THRESHOLD);
  bool pumpShouldBeOn = soilDry && !waterLow;

  setPump(pumpShouldBeOn);

  // --- Read DHT11 ---
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  // --- Serial prints ---
  Serial.print("Soil: ");
  Serial.print(soilValue);

  Serial.print(" | WaterLevel: ");
  Serial.print(waterLevelValue);
  Serial.print(" (");
  Serial.print(waterLow ? "LOW - BUZZER ON" : "OK");
  Serial.print(")");

  Serial.print(" | Light: ");
  Serial.print(isDark ? "DARK (LIGHT ON)" : "BRIGHT (LIGHT OFF)");

  Serial.print(" | Pump: ");
  Serial.print(pumpShouldBeOn ? "ON" : "OFF");

  if (!isnan(hum) && !isnan(temp)) {
    Serial.print(" | Temp: ");
    Serial.print(temp);
    Serial.print(" C | Humidity: ");
    Serial.print(hum);
    Serial.print(" %");
  } else {
    Serial.print(" | DHT11 ERROR");
  }

  Serial.println();

  delay(500);
}
