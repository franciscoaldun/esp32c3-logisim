// Blink: el "hola mundo" de Arduino. El LED rojo de la placa (GPIO8) parpadea.
// En el modelo el tiempo va en camara lenta: delay(500) son 500 ciclos de reloj (~1-2 s en Logisim).

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Blink en GPIO8");
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("encendido, millis = ");
  Serial.println(millis());
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("apagado");
  delay(500);
}
