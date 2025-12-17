#include <DHT.h>

// --- Senzor de umiditate sol ---
const int soilPin = A0;      // pin analogic pentru senzorul de umiditate sol (AOUT)

// --- Releu pompa ---
const int relayPin = 7;      // pin digital pentru control releu pompa
int DRY_THRESHOLD = 400;     // prag sub care solul este considerat uscat
const bool RELAY_ACTIVE_LOW = false; // false = releu activ pe HIGH

// --- Senzor DHT11 (temperatura + umiditate aer) ---
#define DHTPIN 8             // pin digital pentru DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- Senzor nivel apa + buzzer ---
const int waterLevelPin = A1;        // pin analogic pentru senzor nivel apa
const int buzzerPin     = 10;        // pin digital pentru buzzer
const int WATER_LOW_THRESHOLD = 300; // prag sub care nivelul apei este prea mic

// --- Senzor de lumina (HW-072 DO) + bec ---
const int lightDigitalPin = 11; // pin digital pentru iesirea DO a senzorului de lumina
const int ledPin = 12;          // pin pentru bec / LED (aprins cand este intuneric)

int waterLevelValue = 0;

// Functie pentru controlul pompei prin releu
void setPump(bool on) {
  if (RELAY_ACTIVE_LOW)
    digitalWrite(relayPin, on ? LOW : HIGH);  // LOW = pompa pornita
  else
    digitalWrite(relayPin, on ? HIGH : LOW);  // HIGH = pompa pornita
}

void setup() {
  Serial.begin(9600); // initializare comunicatie seriala

  // Initializare releu pompa
  pinMode(relayPin, OUTPUT);
  setPump(false); // pompa oprita la pornire

  // Initializare buzzer
  pinMode(buzzerPin, OUTPUT);
  noTone(buzzerPin); // buzzer oprit

  // Initializare senzor lumina si bec
  pinMode(lightDigitalPin, INPUT_PULLUP); // folosim rezistenta interna pull-up
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // bec stins initial

  // Initializare senzor DHT11
  dht.begin();

  Serial.println("Sistem pornit (pompa + sol + DHT11 + apa + buzzer + lumina)...");
}

void loop() {
  // --- Citire umiditate sol ---
  int soilValue = analogRead(soilPin);

  // --- Citire nivel apa ---
  waterLevelValue = analogRead(waterLevelPin);

  // --- Citire senzor lumina ---
  int lightVal = digitalRead(lightDigitalPin);
  bool isDark = (lightVal == HIGH);   // HIGH inseamna intuneric

  // --- Control bec ---
  digitalWrite(ledPin, isDark ? HIGH : LOW);

  // -----------------------------
  // LOGICA BUZZER
  // -----------------------------
  bool waterLow = (waterLevelValue < WATER_LOW_THRESHOLD);

  if (waterLow) {
    tone(buzzerPin, 1000);     // semnal sonor 1000 Hz cand apa este prea putina
  } else {
    noTone(buzzerPin);         // buzzer oprit
  }

  // -----------------------------
  // LOGICA POMPA
  // -----------------------------
  bool soilDry = (soilValue < DRY_THRESHOLD);
  bool pumpShouldBeOn = soilDry && !waterLow;

  setPump(pumpShouldBeOn);

  // --- Citire temperatura si umiditate aer ---
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  // --- Afisare valori in serial monitor ---
  Serial.print("Sol: ");
  Serial.print(soilValue);

  Serial.print(" | Nivel apa: ");
  Serial.print(waterLevelValue);
  Serial.print(" (");
  Serial.print(waterLow ? "SCAZUT - BUZZER PORNIT" : "OK");
  Serial.print(")");

  Serial.print(" | Lumina: ");
  Serial.print(isDark ? "INTUNERIC (BEC APRINS)" : "LUMINA (BEC STINS)");

  Serial.print(" | Pompa: ");
  Serial.print(pumpShouldBeOn ? "PORNITA" : "OPRITA");

  if (!isnan(hum) && !isnan(temp)) {
    Serial.print(" | Temperatura: ");
    Serial.print(temp);
    Serial.print(" C | Umiditate aer: ");
    Serial.print(hum);
    Serial.print(" %");
  } else {
    Serial.print(" | EROARE DHT11");
  }

  Serial.println();

  delay(500); // intarziere intre citiri
}
