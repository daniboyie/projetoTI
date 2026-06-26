#include <DHT.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <HTTPClient.h>

//-----------------------------------------Sensor de Temperatura/Humidade-------------------------------------------------
#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int waterPin = 34;
const int buzzerPin = 26;
const int waterThreshold = 1000;
const int soundPin = 36;
const int soundThreshold = 2500;
const int buzzerChannel = 0;

//-----------------------------------------LCD (Sem potenciómetro)---------------------------------------------------------
const int pinoContraste = 15;
LiquidCrystal lcd(22, 23, 5, 18, 19, 21); // (rs, enable, d4, d5, d6, d7)

//----------------------------------------Servidor--------------------------------------------------------------------------
const char* HOST = "iot.dei.estg.ipleiria.pt";
const String BASE_PATH = "/ti/ti032/api/api.php";

//-----------------------------------------WIFI-------------------------------------------------------------------------
const char* SSID = "labs";
const char* PASS_WIFI = "1nv3nt@r2023_IPLEIRIA";

//-----------------------------------------Protótipos das funções--------------------------------------------------------
String getFromAPI(String nome);
void post2API(String enviaNome, float enviaValor, String enviaHora);
String getValorFicheiro();

//-----------------------------------------Variável de controlo do último estado de acesso mostrado no LCD-----------------
String ultimoAcessoMostrado = "";

//----------------------------------------------Setup----------------------------------------------------------------------
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

  // Ligação WiFi
  Serial.print("Connecting ");
  Serial.println(SSID);

  lcd.setCursor(0, 0);
  lcd.print("Ligando WiFi...");

  WiFi.begin(SSID, PASS_WIFI);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(250);
  }

  Serial.println("\nWiFi Connected");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Ligado!");
  delay(1000);
  lcd.clear();
}

//-------------------------------------------------------loop---------------------------------------------------------------
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

  // --- Ler estado do acesso RFID vindo do Raspberry, via ficheiro valor.txt ---
  String acesso = getValorFicheiro(); // espera "1" (aceite) ou "0" (rejeitado)

  Serial.print("Estado RFID: ");
  if (acesso == "0") {
    Serial.println("REJEITADO");
  } else if (acesso == "1") {
    Serial.println("ACEITE");
  } else {
    Serial.println("(sem estado / " + acesso + ")");
  }

  // --- LÓGICA DE ALARMES E DISPLAY LCD ---
  if (water > waterThreshold) {
    // Alerta de Inundação no LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! ALERTA !!!");
    lcd.setCursor(0, 1);
    lcd.print("AGUA! W: ");
    lcd.print(water);

    // Tons do Buzzer
    ledcWriteTone(buzzerChannel, 1000);
    delay(500);
    ledcWriteTone(buzzerChannel, 1200);
    delay(300);
    ledcWriteTone(buzzerChannel, 0);
    delay(500);

    ultimoAcessoMostrado = ""; // reset, para o próximo acesso voltar a mostrar no LCD

  } else if (sound > soundThreshold) {
    // Alerta de Ruído no LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! ALERTA !!!");
    lcd.setCursor(0, 1);
    lcd.print("SOM! S: ");
    lcd.print(sound);

    // Tons do Buzzer
    ledcWriteTone(buzzerChannel, 1500);
    delay(500);
    ledcWriteTone(buzzerChannel, 1300);
    delay(300);
    ledcWriteTone(buzzerChannel, 0);
    delay(500);

    ultimoAcessoMostrado = "";

  } else if (acesso == "0" && ultimoAcessoMostrado != "0") {
    // RFID rejeitado -> LCD + som de erro
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ACESSO NEGADO");
    lcd.setCursor(0, 1);
    lcd.print(":(");

    ledcWriteTone(buzzerChannel, 400);
    delay(300);
    ledcWriteTone(buzzerChannel, 0);
    delay(200);

    ultimoAcessoMostrado = "0";
    delay(1500); // tempo para a mensagem ficar visível

  } else if (acesso == "1" && ultimoAcessoMostrado != "1") {
    // RFID aceite -> LCD + som de sucesso
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ACESSO PERMITIDO");
    lcd.setCursor(0, 1);
    lcd.print(":)");

    ledcWriteTone(buzzerChannel, 1800);
    delay(200);
    ledcWriteTone(buzzerChannel, 0);

    ultimoAcessoMostrado = "1";
    delay(1500); // tempo para a mensagem ficar visível

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

    delay(500);
  }
}

//---------------------------------------------------FUNÇÕES----------------------------------------------------------

String getFromAPI(String nome) {
  HTTPClient http;
  String url = "http://" + String(HOST) + BASE_PATH + "?nome=" + nome;

  http.begin(url);
  int httpCode = http.GET();
  String response = "";

  if (httpCode > 0) {
    response = http.getString();
    response.trim();
  }

  http.end();
  return response;
}

void post2API(String enviaNome, float enviaValor, String enviaHora) {
  HTTPClient http;
  String url = "http://" + String(HOST) + BASE_PATH;

  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String body = "nome=" + enviaNome + "&valor=" + String(enviaValor) + "&hora=" + enviaHora;
  http.POST(body);
  http.getString();

  http.end();
}

String getValorFicheiro() {
  HTTPClient http;
  String url = "http://" + String(HOST) + "/ti/ti032/api/files/rfid/valor.txt";

  http.begin(url);
  int httpCode = http.GET();
  String response = "";

  if (httpCode > 0) {
    response = http.getString();
    response.trim();
  }

  http.end();
  return response;
}