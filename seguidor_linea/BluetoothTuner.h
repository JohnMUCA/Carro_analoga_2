#ifndef BLUETOOTH_TUNER_H
#define BLUETOOTH_TUNER_H

#include <Arduino.h>
#include <BluetoothSerial.h>
#include "LineFollower.h"

// Nombre del dispositivo Bluetooth que aparecerá en el celular o PC
const char BT_TUNER_NAME[] = "Carro_Seguidor_PI";

// Estado del sintonizador en tiempo real
struct TunerState {
  bool motorsRunning;   // true = carro avanzando en pista, false = motores frenados
  bool telemetryActive; // true = imprime Pos, Error, Izq, Der cada 150 ms
};

// Funciones públicas
void initBluetoothTuner();
void updateBluetoothTuner(PIDConfig &pid, TunerState &state);
void sendTunerTelemetry(const PIDConfig &pid, const LineCorrection &corr);
void printTunerStatus(const PIDConfig &pid, const TunerState &state);

#endif
