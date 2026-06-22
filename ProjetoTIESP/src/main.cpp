#include <DHT.h>
#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int waterPin = 34;
const int buzzerPin = 26;
const int waterThreshold = 1000;
const int soundPin = 36;
const int soundThreshold = 1000;
const int buzzerChannel = 0;

void setup() {
  Serial.begin(115200);
  ledcSetup(buzzerChannel, 1000, 8);
  ledcAttachPin(buzzerPin, buzzerChannel);
}

void loop() {
  int water = analogRead(waterPin);
  int sound = analogRead(soundPin);

  Serial.print("Water level: ");
  Serial.println(water);
  Serial.print("Sound level: ");
  Serial.println(sound);

  if (water > waterThreshold) {
    ledcWriteTone(buzzerChannel, 1000);

  } else if (sound > soundThreshold) {
    ledcWriteTone(buzzerChannel, 1500);
    delay(500); // diferente do segundo tom
    ledcWriteTone(buzzerChannel, 1300);
    delay(300); // diferente do primeiro tom

  } else {
    ledcWriteTone(buzzerChannel, 0);
  }
}