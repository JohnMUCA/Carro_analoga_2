#ifndef BLUETOOTH_TUNER_H
#define BLUETOOTH_TUNER_H

#include <Arduino.h>
#include <BluetoothSerial.h>
#include "LineFollower.h"
#include "MotorDriver.h"

// Nombre del dispositivo Bluetooth que aparecerá en el celular o PC
const char BT_TUNER_NAME[] = "Carro_Seguidor_PI";

// Estado del sintonizador en tiempo real
struct TunerState {
  bool motorsRunning;   // true = carro avanzando en pista, false = motores frenados
  bool telemetryActive; // true = imprime Pos, Error, Izq, Der cada 150 ms
};

// Inicializa Bluetooth Serial SPP
void initBluetoothTuner();

// Procesa comandos entrantes desde Bluetooth (celular/PC) o USB Serial sin bloquear el loop
void updateBluetoothTuner(PIDConfig &pid, TunerState &state);

// Envía una línea de telemetría tanto por USB como por Bluetooth
void sendTunerTelemetry(const PIDConfig &pid, const LineCorrection &corr);

// Imprime los parámetros actuales y la guía rápida de comandos
void printTunerStatus(const PIDConfig &pid, const TunerState &state);

#endif
