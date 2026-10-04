// Boton con interrupcion: cada vez que presionas BOOT (GPIO9) se ejecuta la funcion al_presionar().
// BOTON_1 (GPIO2) se lee en loop() y enciende el LED de GPIO3 mientras lo mantienes.

volatile int veces = 0;

void al_presionar() {          // rutina de interrupcion (ISR): corta y rapida
  veces++;
}

void setup() {
  Serial.begin(115200);
  pinMode(BOOT_PIN, INPUT_PULLUP);
  pinMode(2, INPUT);
  pinMode(3, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BOOT_PIN), al_presionar, FALLING);
  Serial.println("Presiona BOOT (interrupcion) o BOTON_1 (lectura normal)");
}

int ultimas = 0;

void loop() {
  digitalWrite(3, digitalRead(2));
  if (veces != ultimas) {
    ultimas = veces;
    Serial.print("BOOT presionado ");
    Serial.print(ultimas);
    Serial.println(" veces");
  }
}
