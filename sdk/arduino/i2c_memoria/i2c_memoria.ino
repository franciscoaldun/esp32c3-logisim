// Wire (I2C): escribe y lee la memoria 24C02 de la placa (direccion 0x50, SDA = GPIO4, SCL = GPIO5).
#include <Wire.h>

const uint8_t MEMORIA = 0x50;

void escribir(uint8_t pos, const char *texto) {
  Wire.beginTransmission(MEMORIA);
  Wire.write(pos);
  while (*texto) Wire.write(*texto++);
  Wire.write(0);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("Busco dispositivos I2C...");
  for (uint8_t a = 0x48; a < 0x58; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.print("  responde 0x");
      Serial.println(a, HEX);
    }
  }
  escribir(0x20, "Hola Wire!");
  Wire.beginTransmission(MEMORIA);
  Wire.write(0x20);
  Wire.endTransmission(false);           // sin STOP: la lectura sigue con un "repeated start"
  Wire.requestFrom(MEMORIA, (size_t)11);
  Serial.print("Lei: ");
  while (Wire.available()) {
    char c = Wire.read();
    if (c) Serial.print(c);
  }
  Serial.println();
}

void loop() {
}
