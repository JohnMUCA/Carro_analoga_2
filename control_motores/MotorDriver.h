#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

// --- Parámetros de configuración PWM (LEDC ESP32 v2.x) ---
const uint32_t PWM_FREQ = 1000; // 1 kHz
const uint8_t  PWM_RES  = 8;    // 8 bits (0 - 255)

// --- Asignación de Pines GPIO para 4 Motores ---
// Lado Izquierdo (Frontal y Trasero)
const int PIN_M_FL_IN1 = 33;
const int PIN_M_FL_IN2 = 32;
const int PIN_M_RL_IN1 = 19; // Ajusta según tu conexionado
const int PIN_M_RL_IN2 = 18; // Ajusta según tu conexionado

// Lado Derecho (Frontal y Trasero)
const int PIN_M_FR_IN3 = 25;
const int PIN_M_FR_IN4 = 26;
const int PIN_M_RR_IN3 = 27; // Ajusta según tu conexionado
const int PIN_M_RR_IN4 = 14; // Ajusta según tu conexionado

// --- Asignación de Canales LEDC (0 a 7 disponibles en ESP32 estándar) ---
const uint8_t CH_FL_IN1 = 0;
const uint8_t CH_FL_IN2 = 1;
const uint8_t CH_RL_IN1 = 2;
const uint8_t CH_RL_IN2 = 3;

const uint8_t CH_FR_IN3 = 4;
const uint8_t CH_FR_IN4 = 5;
const uint8_t CH_RR_IN3 = 6;
const uint8_t CH_RR_IN4 = 7;

// --- Funciones públicas del módulo ---
void initMotorDriver();
void setMotorPWM(uint8_t chForward, uint8_t chBackward, int speed);
void setChassisSpeed(int speedLeft, int speedRight);
void stopMotors();

#endif