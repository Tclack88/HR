#include <Wire.h>
#include "MAX30105.h"            // SparkFun MAX3010x Pulse & Proximity Sensor Library
#include "spo2_algorithm.h"      // make sure spo2_algorithm.h/.cpp are in the same folder

MAX30105 particle;

#define WINDOW_SIZE 100  // number of samples per second

uint32_t irBuffer[WINDOW_SIZE];
uint32_t redBuffer[WINDOW_SIZE];

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== MAX30102 Heart Rate & SpO₂ Monitor ===");
  Serial.println("Initializing sensor...\n");

  Wire.begin(21, 22);  // ESP32 default I2C pins

  if (!particle.begin(Wire)) {
    Serial.println("❌ Sensor not found. Check wiring and power.");
    while (1);
  }

  // Configure sensor parameters
  byte ledBrightness = 60;    // 0–255
  byte sampleAverage = 4;
  byte ledMode = 2;           // 2 = Red + IR
  int sampleRate = 100;       // Hz
  int pulseWidth = 411;       // µs
  int adcRange = 16384;       // nA

  particle.setup(ledBrightness, sampleAverage, ledMode,
                 sampleRate, pulseWidth, adcRange);

  Serial.println("✅ Sensor initialized successfully!");
  Serial.println("Place your finger gently on the sensor.\n");
  Serial.println("---------------------------------------------------");
}

void loop() {
  static uint32_t lastCalc = 0;
  static int idx = 0;

  particle.check();

  while (particle.available()) {
    irBuffer[idx]  = particle.getFIFOIR();
    redBuffer[idx] = particle.getFIFORed();
    idx++;
    particle.nextSample();
  }

  // Compute once per second
  if (idx >= WINDOW_SIZE && millis() - lastCalc >= 1000) {
    lastCalc = millis();

    // Calculate average IR value to check finger contact quality
    uint32_t irSum = 0;
    uint32_t irMin = UINT32_MAX;
    uint32_t irMax = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
      irSum += irBuffer[i];
      if (irBuffer[i] < irMin) irMin = irBuffer[i];
      if (irBuffer[i] > irMax) irMax = irBuffer[i];
    }
    uint32_t irAvg = irSum / WINDOW_SIZE;
    uint32_t irRange = irMax - irMin;

    // Print IR stats for debugging
    Serial.print("📊 IR Avg: ");
    Serial.print(irAvg);
    Serial.print("   |   Min: ");
    Serial.print(irMin);
    Serial.print("   |   Max: ");
    Serial.print(irMax);
    Serial.print("   |   Range: ");
    Serial.println(irRange);

    // Check for good sensor contact
    const uint32_t IR_THRESHOLD = 50000;
    const uint32_t RANGE_MAX = 250;
    
    if (irAvg < IR_THRESHOLD) {
      Serial.println("⚠️  **POOR SENSOR READING** - Check finger placement");
    } else {
      int32_t spo2 = 0, heartRate = 0;
      int8_t spo2Valid = 0, hrValid = 0;

      maxim_heart_rate_and_oxygen_saturation(
          irBuffer, WINDOW_SIZE,
          redBuffer,
          &spo2, &spo2Valid,
          &heartRate, &hrValid);

      Serial.print("❤️ Heart Rate: ");
      if (hrValid)
        Serial.print(heartRate);
      else
        Serial.print("--");
      Serial.print(" bpm");

      Serial.print("   |   🩸 SpO₂: ");
      if (spo2Valid)
        Serial.print(spo2);
      else
        Serial.print("--");
      Serial.print("%");
      
      // Indicate degraded signal quality
      if (irRange > RANGE_MAX) {
        Serial.print("   *degraded*");
      }
      
      Serial.println();
    }

    Serial.println("---------------------------------------------------");

    idx = 0;  // reset buffer for next second
  }
}
