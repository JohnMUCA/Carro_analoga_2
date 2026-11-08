#ifndef LINE_FOLLOWER_H
#define LINE_FOLLOWER_H

#include <Arduino.h>
#include "MotorDriver.h"

const uint8_t NUM_SENSORS = 8;

// Velocidad PWM de giro sobre su propio eje durante la autocalibración
const int CALIB_TURN_SPEED = 130;

// Velocidad PWM de giro sobre su propio eje al recuperar esquinas de 90°
const int CORNER_PIVOT_SPEED = 170;

// Pines ADC del ESP32 conectados a los sensores 0 al 7 del QTR-8A
const uint8_t QTR_PINS[NUM_SENSORS] = {14, 27, 26, 25, 33, 32, 35, 34};

// Estructura para configurar las ganancias del PID
struct PIDConfig {
  float Kp;
  float Ki;
  float Kd;             // Término derivativo (anticipa curvas de 90° y amortigua oscilaciones)
  int baseSpeed;        // Velocidad crucero en recta (ej. 55 - 160)
  int topSpeed;         // Límite máximo de PWM en motores en curva (por defecto 255)
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
void autoCalibrateLineSensor(); // Barre izquierda/derecha con los motores y centra el carro en la línea
LineCorrection computeLineCorrection(PIDConfig &pid);
void resetPI();

#endif
