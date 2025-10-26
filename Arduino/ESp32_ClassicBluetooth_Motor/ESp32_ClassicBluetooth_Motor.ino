#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
#define LED_PIN LED_BUILTIN  // Usually GPIO2 (depends on board)
// ==== Pin Motor ====
#define ENA 14
#define IN1 27
#define IN2 26
#define IN3 25
#define IN4 33
#define ENB 32
// ==== LEDC (PWM) Settings ====
#define PWM_FREQ 5000  // 5 kHz PWM frequency
#define PWM_RES 8      // 8-bit resolution (0–255)
#define PWM_CHANNEL_A 0
#define PWM_CHANNEL_B 1


void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Robot");  // Nama Bluetooth yang akan muncul di HP
  Serial.println("Bluetooth siap! Pair dengan ESP32_Robot");
  pinMode(LED_BUILTIN, OUTPUT);
  // === LEDC API in Setup ===
  ledcAttachChannel(ENA, PWM_FREQ, PWM_RES, PWM_CHANNEL_A);  // Motor kiri
  ledcAttachChannel(ENB, PWM_FREQ, PWM_RES, PWM_CHANNEL_B);  // Motor kanan
}

void loop() {
  if (SerialBT.available()) {
    char data = SerialBT.read();
    Serial.println(data);  // Tampilkan di serial monitor

    // Contoh kontrol sederhana
    if (data == 'F') {
      Serial.println("MAJU");
      digitalWrite(LED_BUILTIN, HIGH);
    }
    if (data == 'B') {
      Serial.println("MUNDUR");
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

void stopCar() {
  ledcWriteChannel(PWM_CHANNEL_A, 0);
  ledcWriteChannel(PWM_CHANNEL_B, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void backward() {
  ledcWriteChannel(PWM_CHANNEL_A, speedMotor);
  ledcWriteChannel(PWM_CHANNEL_B, speedMotor);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void forward() {
  ledcWriteChannel(PWM_CHANNEL_A, speedMotor);
  ledcWriteChannel(PWM_CHANNEL_B, speedMotor);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
