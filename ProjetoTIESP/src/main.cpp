#include <DHT.h>
#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
const int waterPin = 34;
const int buzzerPin = 26;
const int waterThreshold = 1000; // ajusta conforme o teu sensor/calibração

const int buzzerChannel = 0; // canal LEDC (0-15)

void setup() {
  Serial.begin(115200);
  ledcSetup(buzzerChannel, 1000, 8);     // canal, frequência inicial, resolução em bits
  ledcAttachPin(buzzerPin, buzzerChannel); // associa o pino ao canal
}

void loop() {
  int water = analogRead(waterPin);
  Serial.println(water);

  if (water > waterThreshold) {
    ledcWriteTone(buzzerChannel, 1000); // buzzer apita a 1000 Hz
  } else {
    ledcWriteTone(buzzerChannel, 0); // desliga o tom
  }

  delay(300);
}