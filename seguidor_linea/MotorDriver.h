#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

// --- Parámetros de configuración PWM (LEDC ESP32 v2.x) ---
const uint32_t PWM_FREQ = 1000; // 1 kHz
const uint8_t  PWM_RES  = 8;    // 8 bits (0 - 255)

// --- Asignación de Pines GPIO para 4 Motores ---
// Lado Izquierdo (Frontal y Trasero)
const int PIN_M_FL_IN3 = 5;  //  <--->
const int PIN_M_FL_IN4 = 4;  //Estan trucados para ajustar el giro
const int PIN_M_RL_IN1 = 15; 
const int PIN_M_RL_IN2 = 2; 

// Lado Derecho (Frontal y Trasero)
const int PIN_M_FR_IN1 = 21;  //  <--->
const int PIN_M_FR_IN2 = 22;  // Estan trucados para ajustar el giro
const int PIN_M_RR_IN3 = 19; 
const int PIN_M_RR_IN4 = 18; 

// --- Asignación de Canales LEDC (0 a 7 disponibles en ESP32 estándar) ---
const uint8_t CH_FL_IN3 = 0;
const uint8_t CH_FL_IN4 = 1;
const uint8_t CH_RL_IN1 = 2;
const uint8_t CH_RL_IN2 = 3;

const uint8_t CH_FR_IN1 = 4;
const uint8_t CH_FR_IN2 = 5;
const uint8_t CH_RR_IN3 = 6;
const uint8_t CH_RR_IN4 = 7;

// --- Funciones públicas del módulo ---
void initMotorDriver();
void setMotorPWM(uint8_t chForward, uint8_t chBackward, int speed);
void setChassisSpeed(int speedLeft, int speedRight);
void stopMotors();

#endif