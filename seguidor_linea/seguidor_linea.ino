#include "MotorDriver.h"
#include "LineFollower.h"

// Configuración del controlador PID para el seguidor de línea
PIDConfig piTune = {
  .Kp = 0.65f,
  .Ki = 0.000f,
  .Kd = 3.250f,
  .baseSpeed = 65,
  .topSpeed = 255,
  .maxControl = 200,
  .integralLimit = 2000.0f
};

unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(1500); // Pausa inicial tras encender para soltar el carro sobre la línea

  Serial.println("=== INICIALIZANDO SEGUIDOR DE LINEA AUTONOMO ===");
  initMotorDriver();
  initLineFollower();

  // Autocalibración automática: el carro gira izquierda/derecha y se centra solo
  Serial.println(">> AUTOCALIBRANDO: El carro girara sobre la linea...");
  autoCalibrateLineSensor();
  Serial.println(">> Autocalibracion finalizada exitosamente. Iniciando seguimiento...");
}

void loop() {
  // 1. Ejecutar cálculo PID
  LineCorrection corr = computeLineCorrection(piTune);

  // 2. Aplicar velocidad a los 4 motores
  setChassisSpeed(corr.speedLeft, corr.speedRight);

  // 3. Imprimir telemetría periódica por Monitor Serie
  if (millis() - lastPrint >= 100) {
    lastPrint = millis();
    Serial.printf("Pos: %4d | Error: %5d | Izq: %4d | Der: %4d | Perdio: %s\n",
                  corr.position,
                  corr.error,
                  corr.speedLeft,
                  corr.speedRight,
                  corr.lineLost ? "SI" : "NO");
  }

  delay(10); // Lazo de control a ~100 Hz
}
