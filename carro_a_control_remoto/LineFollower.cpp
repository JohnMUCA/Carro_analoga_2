#include "LineFollower.h"

// Arreglos para la calibración automática de mínimos y máximos por canal
static uint16_t sensorMin[NUM_SENSORS];
static uint16_t sensorMax[NUM_SENSORS];

// Variables del lazo PID
static float integralError = 0.0f;
static int lastError = 0;
static float filteredDerivative = 0.0f;
static unsigned long lastComputeTime = 0;
static int lastValidPosition = 3500;

// Declaración adelantada para usarla en el autocentrado
static int readLinePosition(bool &lineLost);

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
  lastError = 0;
  filteredDerivative = 0.0f;
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

// Ejecuta un giro durante 'durationMs' mientras muestrea los sensores QTR-8A
static void sweepAndSample(int speedLeft, int speedRight, unsigned long durationMs) {
  setChassisSpeed(speedLeft, speedRight);
  unsigned long start = millis();
  while (millis() - start < durationMs) {
    calibrateLineSensor();
    delay(5);
  }
}

void autoCalibrateLineSensor() {
  // 1. Reiniciar mínimos y máximos antes de iniciar el barrido
  initLineFollower();

  // 2. Barrido oscilatorio sobre el propio eje (Derecha -> Izquierda -> Derecha -> Izquierda)
  sweepAndSample( CALIB_TURN_SPEED, -CALIB_TURN_SPEED, 550);  // Medio giro a la derecha
  stopMotors();
  delay(80);

  sweepAndSample(-CALIB_TURN_SPEED,  CALIB_TURN_SPEED, 1100); // Barrido completo a la izquierda
  stopMotors();
  delay(80);

  sweepAndSample( CALIB_TURN_SPEED, -CALIB_TURN_SPEED, 1100); // Barrido completo a la derecha
  stopMotors();
  delay(80);

  sweepAndSample(-CALIB_TURN_SPEED,  CALIB_TURN_SPEED, 1100); // Barrido completo a la izquierda
  stopMotors();
  delay(80);

  // 3. Respaldo de seguridad: si algún sensor extremo no alcanzó a pisar la línea
  //    (rango < 350), hereda el blanco mínimo y negro máximo globales medidos.
  uint16_t globalMin = 4095;
  uint16_t globalMax = 0;
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    if (sensorMin[i] < globalMin) globalMin = sensorMin[i];
    if (sensorMax[i] > globalMax) globalMax = sensorMax[i];
  }
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    if (sensorMax[i] <= sensorMin[i] || (sensorMax[i] - sensorMin[i]) < 350) {
      sensorMin[i] = globalMin;
      sensorMax[i] = globalMax;
    }
  }

  // 4. Auto-centrado final: regresa girando a la derecha y se frena apenas centra la línea
  setChassisSpeed(CALIB_TURN_SPEED, -CALIB_TURN_SPEED);
  unsigned long centerStart = millis();
  while (millis() - centerStart < 1000) {
    bool lost = false;
    int pos = readLinePosition(lost);
    if (!lost && pos >= 2700 && pos <= 4300) {
      break; // Línea centrada bajo el sensor
    }
    delay(5);
  }

  stopMotors();
  resetPI();
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

  if (dt <= 0.0f || dt > 0.1f) dt = 0.01f; // Protección ante retrasos temporales (~100 Hz)

  // 1. Obtener posición y calcular error respecto al centro (3500)
  result.position = readLinePosition(result.lineLost);
  result.error = result.position - 3500;

  int limit = (pid.topSpeed > 0 && pid.topSpeed <= 255) ? pid.topSpeed : 255;

  // CASO ESPECIAL ESQUINA 90°: Si el carro se pasa unos milímetros del vértice de 90°
  // y pierde la línea, frena el avance longitudinal y pivota sobre su propio eje
  // hacia el lado donde vio la línea por última vez hasta reengancharla.
  if (result.lineLost) {
    int pivot = constrain(max(pid.baseSpeed * 2, CORNER_PIVOT_SPEED), 90, limit);
    if (result.position == 0) {
      // La línea quedó a la izquierda -> pivotar sobre el eje a la izquierda
      result.speedLeft  = -pivot;
      result.speedRight =  pivot;
    } else {
      // La línea quedó a la derecha -> pivotar sobre el eje a la derecha
      result.speedLeft  =  pivot;
      result.speedRight = -pivot;
    }
    lastError = result.error;
    return result;
  }

  // 2. Cálculo del término Integral con protección Anti-Windup
  //    En curvas muy cerradas (|error| > 2000), congelamos o limpiamos la integral
  //    para que no cause sobreimpulso al salir de una curva de 90°.
  if (abs(result.error) < 2000) {
    integralError += (result.error * dt);
    integralError = constrain(integralError, -pid.integralLimit, pid.integralLimit);
  } else {
    integralError = 0.0f;
  }

  // 3. Cálculo del término Derivativo (sobre el cambio de error por ciclo, filtrado)
  float rawDerivative = (float)(result.error - lastError);
  filteredDerivative = 0.7f * rawDerivative + 0.3f * filteredDerivative;
  lastError = result.error;

  // 4. Ecuación del controlador PID
  float pTerm = pid.Kp * (float)result.error;
  float iTerm = pid.Ki * integralError;
  float dTerm = pid.Kd * filteredDerivative;
  int controlSignal = (int)(pTerm + iTerm + dTerm);

  // Limitar señal de corrección
  controlSignal = constrain(controlSignal, -pid.maxControl, pid.maxControl);

  // 5. Reducción dinámica de velocidad de avance en curvas cerradas (|error| > 1800):
  //    Si la línea llega a los sensores de los extremos (curva de 90°), reduce la
  //    velocidad de avance hacia 0 para que el carro gire sobre su eje en lugar de seguir de largo.
  int effectiveBase = pid.baseSpeed;
  int absErr = abs(result.error);
  if (absErr > 1800) {
    effectiveBase = map(absErr, 1800, 3500, pid.baseSpeed, 0);
    effectiveBase = constrain(effectiveBase, 0, pid.baseSpeed);
  }

  // 6. Mapeo a los motores izquierdo y derecho
  result.speedLeft  = constrain(effectiveBase + controlSignal, -limit, limit);
  result.speedRight = constrain(effectiveBase - controlSignal, -limit, limit);

  return result;
}
