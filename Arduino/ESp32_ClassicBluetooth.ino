#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Robot"); // Nama Bluetooth yang akan muncul di HP
  Serial.println("Bluetooth siap! Pair dengan ESP32_Robot");
}

void loop() {
  if (SerialBT.available()) {
    char data = SerialBT.read();
    Serial.println(data); // Tampilkan di serial monitor

    // Contoh kontrol sederhana
    if (data == 'F') Serial.println("MAJU");
    if (data == 'B') Serial.println("MUNDUR");
  }
}
