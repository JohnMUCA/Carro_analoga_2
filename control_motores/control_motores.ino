#include "MotorDriver.h"

void setup() {
  Serial.begin(115200);
  initMotorDriver();
  delay(1000);
  Serial.println("--- MODO CALIBRACION RUEDA POR RUEDA ---");
}

void loop() {
  // 1. Frontal Izquierda
  Serial.println("Probando: FRONT-LEFT (Adelante)");
  setMotorPWM(CH_FL_IN1, CH_FL_IN2, 180);
  delay(2000);
  stopMotors();
  delay(1000);

  // 2. Trasera Izquierda
  Serial.println("Probando: REAR-LEFT (Adelante)");
  setMotorPWM(CH_RL_IN1, CH_RL_IN2, 180);
  delay(2000);
  stopMotors();
  delay(1000);

  // 3. Frontal Derecha
  Serial.println("Probando: FRONT-RIGHT (Adelante)");
  setMotorPWM(CH_FR_IN3, CH_FR_IN4, 180);
  delay(2000);
  stopMotors();
  delay(1000);

  // 4. Trasera Derecha
  Serial.println("Probando: REAR-RIGHT (Adelante)");
  setMotorPWM(CH_RR_IN3, CH_RR_IN4, 180);
  delay(2000);
  stopMotors();
  delay(3000);
}