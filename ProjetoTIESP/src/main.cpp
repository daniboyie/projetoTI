#include <Arduino.h>

// put function declarations here:
const int sensorPin = 34; // exemplo

void setup() {
  Serial.begin(115200);
}

void loop() {
  int valor = analogRead(sensorPin); // 0-4095 no ESP32
  Serial.println(valor);
  
}