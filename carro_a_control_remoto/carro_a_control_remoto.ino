#include "MotorDriver.h"
#include "GamepadController.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Inicialización de módulos
  initMotorDriver();
  initGamepad();
}

void loop() {
  GamepadInput pad = readGamepad();

  if (pad.connected) {
    // Cinemática diferencial (Arcade Drive):
    // Giro a la derecha (steering > 0) -> Rueda izquierda acelera, derecha reduce
    // Giro a la izquierda (steering < 0) -> Rueda izquierda reduce, derecha acelera
    int speedLeft  = pad.throttle + pad.steering;
    int speedRight = pad.throttle - pad.steering;

    // Normalizar si se sobrepasa el límite PWM (-255 a 255)
    speedLeft  = constrain(speedLeft, -255, 255);
    speedRight = constrain(speedRight, -255, 255);

    setChassisSpeed(speedLeft, speedRight);

    delay(20); // Ciclo de control fluido (~50 Hz)
  } else {
    // Failsafe: Si el control se desconecta o no enlaza, frenar motores
    stopMotors();
    delay(100);
  }
}