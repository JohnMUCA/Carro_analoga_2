#include "MotorDriver.h"
#include "LineFollower.h"
#include "BluetoothTuner.h"

// Configuración inicial del controlador PID
PIDConfig piTune = {
  .Kp = 0.65f,
  .Ki = 0.000f,
  .Kd = 3.250f,
  .baseSpeed = 65,
  .topSpeed = 255,
  .maxControl = 200,
  .integralLimit = 2000.0f
};

TunerState tunerState = {
  .motorsRunning = false, // Inicia detenido; envía 'go' o '1' por Bluetooth/Serial para arrancar
  .telemetryActive = true
};

unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(1500); // Pausa inicial tras encender para soltar el carro sobre la línea

  Serial.println("=== INICIALIZANDO SEGUIDOR DE LINEA + TUNER BLUETOOTH ===");
  initMotorDriver();
  initLineFollower();

  // Fase de Autocalibración: El carro gira izquierda/derecha sobre la línea y se centra solo
  Serial.println(">> AUTOCALIBRANDO: El carro girara sobre la linea...");
  autoCalibrateLineSensor();
  Serial.println(">> Autocalibracion y centrado finalizados exitosamente.");

  initBluetoothTuner();
  printTunerStatus(piTune, tunerState);
}

void loop() {
  // 1. Leer comandos en tiempo real desde Bluetooth ("Carro_Seguidor_PI") o Monitor Serie
  updateBluetoothTuner(piTune, tunerState);

  // 2. Ejecutar el cálculo PID
  LineCorrection corr = computeLineCorrection(piTune);

  // 3. Aplicar velocidad a los motores solo si está en marcha ('go')
  if (tunerState.motorsRunning) {
    setChassisSpeed(corr.speedLeft, corr.speedRight);
  } else {
    stopMotors();
  }

  // 4. Imprimir telemetría cada 150 ms
  if (tunerState.telemetryActive && (millis() - lastPrint >= 150)) {
    lastPrint = millis();
    sendTunerTelemetry(piTune, corr);
  }

  delay(10); // Frecuencia de muestreo de ~100 Hz
}
