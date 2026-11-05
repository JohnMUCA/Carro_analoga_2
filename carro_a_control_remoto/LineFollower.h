#ifndef LINE_FOLLOWER_H
#define LINE_FOLLOWER_H

#include <Arduino.h>

const uint8_t NUM_SENSORS = 8;

// Pines ADC del ESP32 conectados a los sensores 0 al 7 del QTR-8A
const uint8_t QTR_PINS[NUM_SENSORS] = {14, 27, 26, 25, 33, 32, 35, 34}; 

// Estructura para configurar las ganancias del PI
struct PIDConfig {
  float Kp;
  float Ki;
  int baseSpeed;        // Velocidad crucero hacia adelante (ej. 140 - 200)
  int maxControl;       // Límite máximo de corrección de giro (ej. 150 - 255)
  float integralLimit;  // Límite anti-windup para el término integral
};

// Estructura con el resultado del cálculo
struct LineCorrection {
  int speedLeft;
  int speedRight;
  int position;
  int error;
  bool lineLost;
};

// Funciones públicas
void initLineFollower();
void calibrateLineSensor(); // Debe ejecutarse pasando el sensor sobre línea y fondo
LineCorrection computeLineCorrection(PIDConfig &pid);
void resetPI();

#endif
