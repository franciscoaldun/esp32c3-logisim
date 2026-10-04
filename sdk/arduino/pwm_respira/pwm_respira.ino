// analogWrite: PWM con el periferico LEDC. El LED de GPIO18 sube y baja de "brillo".
// Los LEDs de Logisim son de encendido/apagado: el brillo se ve como la fraccion del tiempo encendido.

int brillo = 0;
int paso = 32;

void setup() {
  Serial.begin(115200);
  Serial.println("PWM en GPIO18 (y fijo al 25% en GPIO0)");
  analogWrite(0, 64);
}

void loop() {
  analogWrite(18, brillo);
  Serial.printf("brillo = %3d/255\n", brillo);
  brillo += paso;
  if (brillo >= 255 || brillo <= 0) {
    paso = -paso;
    brillo = constrain(brillo, 0, 255);
  }
  delay(800);
}
