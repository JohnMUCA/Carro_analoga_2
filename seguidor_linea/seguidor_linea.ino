#include "LineFollower.h"

// Definición de las constantes iniciales del controlador PI
PIDConfig piTune = {
  .Kp = 0.045f,         // Iniciar con valores bajos (ej: 0.03 a 0.06)
  .Ki = 0.005f,         // Término integral pequeño
  .baseSpeed = 160,     // Velocidad base
  .maxControl = 200,    // Límite de giro
  .integralLimit = 2000.0f // Límite anti-windup
};

unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== INICIALIZANDO SENSOR QTR-8A ===");
  initLineFollower();

  // Fase de Calibración: Durante 6 segundos desliza el sensor de lado a lado sobre la pista
  Serial.println(">> CALIBRANDO: Mueve el sensor sobre la línea y el fondo por 6 segundos...");
  unsigned long calibStart = millis();
  while (millis() - calibStart < 6000) {
    calibrateLineSensor();
    delay(10);
  }
  Serial.println(">> Calibración finalizada exitosamente.\n");
}

void loop() {
  // Ejecutar el cálculo PI
  LineCorrection corr = computeLineCorrection(piTune);

  // Imprimir telemetría cada 100 ms para inspeccionar el comportamiento
  if (millis() - lastPrint >= 100) {
    lastPrint = millis();

    Serial.printf("Pos: %4d | Error: %5d | Izq: %4d | Der: %4d | Perdió: %s\n",
                  corr.position,
                  corr.error,
                  corr.speedLeft,
                  corr.speedRight,
                  corr.lineLost ? "SI" : "NO");
  }

  delay(10); // Frecuencia de muestreo de ~100 Hz
}