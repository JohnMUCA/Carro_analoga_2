#include "MotorDriver.h"

int speed_prueba = 100;

void setup() {
  Serial.begin(115200);
  
  initMotorDriver();
  
  delay(1000);
  Serial.println("Driver 4WD inicializado (LEDC v2.x). Iniciando pruebas...");
}

void loop() {
  // 1. Adelante
  Serial.println(">> Adelante");
  setChassisSpeed(speed_prueba, speed_prueba);
  delay(1500);

  stopMotors();
  delay(500);

  // 2. Atrás
  Serial.println(">> Atras");
  setChassisSpeed(-speed_prueba, -speed_prueba);
  delay(1500);

  stopMotors();
  delay(500);

  // 3. Giro sobre su propio eje a la derecha
  Serial.println(">> Giro Derecha");
  setChassisSpeed(speed_prueba, -speed_prueba);
  delay(1500);

  stopMotors();
  delay(500);

  // 4. Giro sobre su propio eje a la izquierda
  Serial.println(">> Giro Izquierda");
  setChassisSpeed(-speed_prueba, speed_prueba);
  delay(1500);

  Serial.println(">> Fin ciclo. Esperando 3s...\n");
  stopMotors();
  delay(3000);
}