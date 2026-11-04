#include "LineFollower.h"

// Arreglos para la calibración automática de mínimos y máximos por canal
static uint16_t sensorMin[NUM_SENSORS];
static uint16_t sensorMax[NUM_SENSORS];

// Variables del lazo integral
static float integralError = 0.0f;
static unsigned long lastComputeTime = 0;
static int lastValidPosition = 3500;

void initLineFollower() {
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    pinMode(QTR_PINS[i], INPUT);
    sensorMin[i] = 4095;
    sensorMax[i] = 0;
  }
  resetPI();
}

void resetPI() {
  integralError = 0.0f;
  lastComputeTime = millis();
}

void calibrateLineSensor() {
  // Tomar lecturas para registrar valores de blanco y negro
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    uint16_t value = analogRead(QTR_PINS[i]);
    if (value < sensorMin[i]) sensorMin[i] = value;
    if (value > sensorMax[i]) sensorMax[i] = value;
  }
}

// Lee los sensores, normaliza las lecturas de 0 a 1000 y calcula el centro ponderado
static int readLinePosition(bool &lineLost) {
  uint32_t weightedSum = 0;
  uint32_t valueSum = 0;
  bool onLine = false;

  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    uint16_t raw = analogRead(QTR_PINS[i]);

    // Escalar lectura entre 0 (fondo blanco) y 1000 (línea negra)
    int normalized = 0;
    if (sensorMax[i] > sensorMin[i]) {
      normalized = map(raw, sensorMin[i], sensorMax[i], 0, 1000);
      normalized = constrain(normalized, 0, 1000);
    }

    // Umbral de detección de presencia de línea
    if (normalized > 200) {
      onLine = true;
    }

    weightedSum += (uint32_t)normalized * (i * 1000);
    valueSum += normalized;
  }

  // Si ningún sensor ve la línea, mantiene la última tendencia de giro
  if (!onLine || valueSum == 0) {
    lineLost = true;
    return (lastValidPosition < 3500) ? 0 : 7000;
  }

  lineLost = false;
  lastValidPosition = weightedSum / valueSum;
  return lastValidPosition;
}

LineCorrection computeLineCorrection(PIDConfig &pid) {
  LineCorrection result;
  unsigned long now = millis();
  float dt = (now - lastComputeTime) / 1000.0f;
  lastComputeTime = now;

  if (dt <= 0.0f || dt > 0.1f) dt = 0.02f; // Protección ante retrasos temporales

  // 1. Obtener posición y calcular error respecto al centro (3500)
  result.position = readLinePosition(result.lineLost);
  result.error = result.position - 3500;

  // 2. Cálculo del término Integral con protección Anti-Windup
  integralError += (result.error * dt);
  integralError = constrain(integralError, -pid.integralLimit, pid.integralLimit);

  // 3. Ecuación del controlador PI
  float pTerm = pid.Kp * (float)result.error;
  float iTerm = pid.Ki * integralError;
  int controlSignal = (int)(pTerm + iTerm);

  // Limitar señal de corrección
  controlSignal = constrain(controlSignal, -pid.maxControl, pid.maxControl);

  // 4. Mapeo a los motores izquierdo y derecho
  result.speedLeft  = constrain(pid.baseSpeed + controlSignal, -255, 255);
  result.speedRight = constrain(pid.baseSpeed - controlSignal, -255, 255);

  return result;
}