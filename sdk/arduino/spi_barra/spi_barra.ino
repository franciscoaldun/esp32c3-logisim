// SPI: manda bytes al 74HC595 de la placa, que los muestra en la barra de 8 LEDs amarillos.
#include <SPI.h>

void setup() {
  Serial.begin(115200);
  SPI.begin();
  Serial.println("SPI -> 74HC595: luz que va y vuelve");
}

void loop() {
  for (int i = 0; i < 8; i++) {
    SPI.transfer(1 << i);
    delay(150);
  }
  for (int i = 6; i > 0; i--) {
    SPI.transfer(1 << i);
    delay(150);
  }
}
