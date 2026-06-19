#include <DHT.h>

#define DHTPIN 27
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const int waterPin = 34;

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  int water = analogRead(waterPin);

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Erro ao ler do DHT11!");
  } else {
    Serial.printf("Temp: %.1f°C | Humidade: %.1f%% | Water: %d\n", temperature, humidity, water);
  }

}