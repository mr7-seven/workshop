#if defined(ESP8266)
#include <ESP8266WiFi.h>
#define LED_PIN D2       // GPIO4 pada NodeMCU

#elif defined(ESP32)
#include <WiFi.h>
#define LED_PIN 2        // GPIO2 bawaan ESP32 (LED biru onboard)
#endif

#include <PubSubClient.h>
#include <ArduinoJson.h>

// ====================== Konfigurasi =========================
const char* ssid        = "...";
const char* password    = "...";
const char* mqtt_server = "broker.emqx.io";
const int   mqtt_port   = 1883;
const char* mqtt_topic  = "ulm/sensor";
const char* mqtt_cmd    = "ulm/relay";

// Jika broker butuh auth, isi di sini
const char* mqtt_user   = "";
const char* mqtt_pass   = "";

WiFiClient espClient;
PubSubClient client(espClient);

// =================== Variabel Global ========================
unsigned long lastReconnectAttempt = 0;
unsigned long lastSend = 0;
const unsigned long interval = 5000;  // setiap 30 detik

// ======================= WiFi Setup =========================
void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print(F("Menghubungkan ke WiFi"));

  unsigned long startAttemptTime = millis();

  // timeout 20 detik
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\nWiFi terhubung!"));
    Serial.print(F("IP Address: "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("\nGagal terhubung WiFi, restart..."));
    ESP.restart();
  }
}

// ===================== MQTT Callback ========================
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(F("Pesan diterima ["));
  Serial.print(topic);
  Serial.print(F("]: "));

  // tampilkan payload mentah
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // simpan ke buffer
  const size_t bufferSize = 256;
  if (length >= bufferSize) {
    Serial.println(F("Payload terlalu panjang, abaikan."));
    return;
  }

  char message[bufferSize];
  memcpy(message, payload, length);
  message[length] = '\0';

  // parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print(F("deserializeJson() gagal: "));
    Serial.println(error.f_str());
    return;
  }

  // contoh: ambil nama & perintah
  String name = doc["name"] | "";
  bool cmd    = doc["cmd"] | false;

  name.trim();

  Serial.print(F("Perintah untuk perangkat: "));
  Serial.println(name);

  // contoh aksi kontrol LED
  if (name == "led") {
    digitalWrite(LED_PIN, cmd ? HIGH : LOW);
    Serial.println(cmd ? F("LED ON") : F("LED OFF"));
  }
}

// ============== MQTT Non-blocking Reconnect =================
bool reconnect() {
  String clientId = "ESPClient-";
#if defined(ESP8266)
  clientId += String(ESP.getChipId(), HEX);
#elif defined(ESP32)
  clientId += String((uint32_t)ESP.getEfuseMac(), HEX);
#endif

  Serial.print(F("MQTT: mencoba koneksi..."));
  if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
    client.subscribe(mqtt_cmd);
    Serial.println(F(" terhubung & subscribed!"));
  } else {
    Serial.print(F(" gagal, rc="));
    Serial.println(client.state());
  }
  return client.connected();
}

// ==================== Kirim Data JSON =======================
void kirimData() {
  StaticJsonDocument<128> doc;

  doc["temp"] = random(20, 30);
  doc["hum"]  = random(40, 80);

  char payload[128];
  size_t len = serializeJson(doc, payload);

  client.publish(mqtt_topic, payload, len);

  Serial.print(F("Data dikirim: "));
  Serial.println(payload);
}

// ===================== Setup Awal ===========================
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // LED mati saat awal

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  lastReconnectAttempt = 0;
  delay(1000);
}

// ======================== Loop ==============================
void loop() {
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();

    unsigned long now = millis();
    if (now - lastSend > interval) {
      lastSend = now;
      kirimData();
    }
  }
}
