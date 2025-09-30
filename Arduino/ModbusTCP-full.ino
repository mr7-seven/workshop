#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #define LED_PIN     LED_BUILTIN   // Usually GPIO2
  #define BUTTON_PIN  0             // GPIO0 = BOOT button
  #define ADC_PIN     A0            // Only one ADC (0–1V)
#else
  #include <WiFi.h>
  #define LED_PIN     LED_BUILTIN   // Usually GPIO2 (depends on board)
  #define BUTTON_PIN  0             // GPIO0 = BOOT button
  #define ADC1_PIN    34            // ADC channel 1
  #define ADC2_PIN    35            // ADC channel 2
  #define ADC3_PIN    36            // ADC channel 3
#endif

#include <ModbusIP_ESP8266.h>

ModbusIP mb;

// WiFi credentials
const char* ssid     = "your_ssid";
const char* password = "your_password";

// --- Modbus Register Offsets ---
const int COIL_BASE = 10;   // Coils start at address 10 (FC1, FC5, FC15)
const int ISTS_BASE = 20;   // Discrete Inputs start at address 20 (FC2)
const int HREG_BASE = 30;   // Holding Registers start at address 30 (FC3, FC6, FC16)
const int IREG_BASE = 40;   // Input Registers start at address 40 (FC4)

unsigned long ts;

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  // Start Modbus TCP server
  mb.server();

  // Add 3 registers of each type
  mb.addCoil(COIL_BASE, 0, 3);   // Coils [10..12]
  mb.addIsts(ISTS_BASE, 0, 3);   // Discrete Inputs [20..22]
  mb.addHreg(HREG_BASE, 0, 3);   // Holding Registers [30..32]
  mb.addIreg(IREG_BASE, 0, 3);   // Input Registers [40..42]

  // Pin setup
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  ts = millis();
}

void loop() {
  mb.task();

  // Update every 1 second
  if (millis() - ts > 1000) {
    ts = millis();

    // --- Discrete Inputs (FC2) ---
    // Map BOOT button to Ists[20]
    mb.Ists(ISTS_BASE + 0, !digitalRead(BUTTON_PIN));
    // mb.Ists(ISTS_BASE + 1, HIGH);
    // mb.Ists(ISTS_BASE + 2, LOW);

    // --- Input Registers (FC4) ---
    mb.Ireg(IREG_BASE + 0, analogRead(ADC_PIN));
    mb.Ireg(IREG_BASE + 1, random(0, 255));
    mb.Ireg(IREG_BASE + 2, random(0, 255));

    // --- Holding Registers (FC3, FC6, FC16) ---
    // Increment Hreg[30] as counter
    uint16_t counter = mb.Hreg(HREG_BASE + 0);
    mb.Hreg(HREG_BASE + 0, (counter + 1)%1000);
    // Keep other holding registers free for master write
  }

  // --- Coils (FC1, FC5, FC15) ---
  // Coil[10] controls LED
  digitalWrite(LED_PIN, mb.Coil(COIL_BASE + 0));
  // Other coils [11], [12] are available for master write
}
