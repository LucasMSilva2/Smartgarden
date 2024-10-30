#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configurações do LCD
// Define os pinos de conexão entre o Arduino e o Display LCD 
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; 
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int sensorUmidadePin = A0;  // Pino do sensor de umidade
const int pinoSensor = A0;
const int valvulaPin = 10;          // Pino da válvula solenoide
const int ledVerdePin = 9;         // Pino do LED verde
const int ledVermelhoPin = 8;     // Pino do LED vermelho

// Configurações do MQTT
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* mqttServer = "YOUR_MQTT_SERVER";
const int mqttPort = 1883;
const char* mqttUser = "YOUR_MQTT_USER";
const char* mqttPassword = "YOUR_MQTT_PASSWORD";

WiFiClient espClient;
PubSubClient client(espClient);

// Limites e tempos
const int limiarSeco = 74;  // Umidade mínima para regar
const unsigned long tempoRega = 500;  // Tempo de rega em milissegundos
unsigned long tempoUltimaRega = 0;

void setup() {
  // Configuração dos pinos
  pinMode(valvulaPin, OUTPUT);
  digitalWrite(pinoValvula, HIGH); // Desliga a válvula
  pinMode(ledVerdePin, OUTPUT);
  pinMode(ledVermelhoPin, OUTPUT);
  lcd.begin(16, 2);
  lcd.print("SmartGarden ");
  Serial.begin(9600);
  connectWiFi();
  client.setServer(mqttServer, mqttPort);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  int umidade = analogRead(sensorUmidadePin);
  int umidadePercent = map(umidade, 0, 1023, 0, 100);
  
  lcd.setCursor(0, 0);
  lcd.print("Umidade: " + String(umidadePercent) + "%");
  
  Regando(umidadePercent);
  
  delay(1000);  // Tempo entre as leituras
}

void Regando(int umidadePercent) {
  if (umidadePercent < limiarSeco) {
    digitalWrite(ledVerdePin, HIGH);  // Acende LED verde
    digitalWrite(ledRed, LOW); // Desliga o LED vermelho
    digitalWrite(valvulaPin, HIGH);    // Abre a válvula
    lcd.setCursor(0, 1);
    lcd.print("Regando...   ");
    tempoUltimaRega = millis();
    
    // Envia informações para o ESP-01
    client.publish("irrigacao/estado", "Regando");
    client.publish("irrigacao/umidade", String(umidadePercent).c_str());
    
    while (millis() - tempoUltimaRega < tempoRega) {
      // Aguarda o tempo de rega
    }

    digitalWrite(valvulaPin, LOW);  // Fecha a válvula
    digitalWrite(ledVerdePin, LOW);  // Apaga LED verde
    digitalWrite(ledVermelhoPin, HIGH);  // Acende LED vermelho
    lcd.setCursor(0, 1);
    lcd.print("Rega concluida");
    
  } else {
    digitalWrite(ledVermelhoPin, HIGH);  // Acende LED vermelho
    digitalWrite(ledVerdePin, LOW);  // Apaga LED verde
    lcd.setCursor(0, 1);
    lcd.print("Solo Encharcado");
    
    // Envia informações para o ESP-01
    client.publish("irrigacao/estado", "Solo Encharcado");
    client.publish("irrigacao/umidade", String(umidadePercent).c_str());
    
    digitalWrite(valvulaPin, LOW);  // Garante que a válvula esteja fechada
  }
}
  
void connectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Conectando ao WiFi...");
    WiFi.begin(ssid, password);
    delay(5000);
  }
  Serial.println("Conectado ao WiFi");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial.println("Conectado ao MQTT");
    } else {
      Serial.print("Falha na conexão. Código de erro: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}
