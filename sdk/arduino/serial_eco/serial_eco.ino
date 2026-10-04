// Eco por el puerto serie: escribe en el teclado de la placa y el ESP32-C3 responde.

String linea = "";

void setup() {
  Serial.begin(115200);
  Serial.println("Escribe algo y presiona Enter:");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (linea.length() > 0) {
        Serial.println();
        linea.toUpperCase();
        Serial.println("Me escribiste: " + linea + " (" + String(linea.length()) + " letras)");
        linea = "";
      }
    } else {
      Serial.print(c);
      linea += c;
    }
  }
}
