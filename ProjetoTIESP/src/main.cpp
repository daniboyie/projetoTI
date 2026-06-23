#include <DHT.h>
#include <LiquidCrystal.h> // Adicionado para o LCD

#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int waterPin = 34;
const int buzzerPin = 26;
const int waterThreshold = 1000;
const int soundPin = 36;
const int soundThreshold = 2500;
const int buzzerChannel = 0;

// Configuração do LCD (Sem potenciómetro)
const int pinoContraste = 15; 
LiquidCrystal lcd(22, 23, 5, 18, 19, 21); // (rs, enable, d4, d5, d6, d7)

void setup() {
  Serial.begin(115200);
  
  // Configuração do Buzzer
  ledcSetup(buzzerChannel, 1000, 8);
  ledcAttachPin(buzzerPin, buzzerChannel);
  
  // Inicialização do DHT
  dht.begin();

  // Configuração do Contraste e Inicialização do LCD
  pinMode(pinoContraste, OUTPUT);
  analogWrite(pinoContraste, 10); // Se o ecrã ficar apagado/escuro, ajusta este valor (0-255)
  lcd.begin(16, 2);
  
  // Mensagem de boas-vindas
  lcd.print("Sistema Pronto!");
  delay(1500);
  lcd.clear();
}

void loop() {
  int water = analogRead(waterPin);
  int sound = analogRead(soundPin);
  
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Print no Serial Monitor
  Serial.print("Water level: ");
  Serial.println(water);
  Serial.print("Sound level: ");
  Serial.println(sound);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // --- LÓGICA DE ALARMES E DISPLAY LCD ---
  
  if (water > waterThreshold) {
    // Alerta de Inundação no LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! ALERTA !!!");
    lcd.setCursor(0, 1);
    lcd.print("AGUA! W: ");
    lcd.print(water);

    // Tons do Buzzer (Teu código original)
    ledcWriteTone(buzzerChannel, 1000);
    delay(500); 
    ledcWriteTone(buzzerChannel, 1200);
    delay(300); 
    ledcWriteTone(buzzerChannel, 0);
    delay(500); 
    
   } else if (sound > soundThreshold) {
    // Alerta de Ruído no LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! ALERTA !!!");
    lcd.setCursor(0, 1);
    lcd.print("SOM! S: ");
    lcd.print(sound);

    // Tons do Buzzer (Teu código original)
    ledcWriteTone(buzzerChannel, 1500);
    delay(500); 
    ledcWriteTone(buzzerChannel, 1300);
    delay(300); 
    ledcWriteTone(buzzerChannel, 0);
    delay(500); 
    
  } else {
    // Se tudo estiver normal, desliga o buzzer
    ledcWriteTone(buzzerChannel, 0);
    
    // Mostra os dados dos sensores no LCD
    // Linha 1: Temperatura e Humidade
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(isnan(temperature) ? 0 : temperature, 1); // Garante que não crasha se o DHT falhar
    lcd.print("C H:");
    lcd.print(isnan(humidity) ? 0 : humidity, 1);
    lcd.print("% "); 

    // Linha 2: Nível de Água e Som
    lcd.setCursor(0, 1);
    lcd.print("W:");
    lcd.print(water);
    lcd.print("  S:");
    lcd.print(sound);
    lcd.print("    "); // Espaços extra para "limpar" resíduos de números maiores anteriores
    
    delay(500); // Delay original do teu loop
  }
}