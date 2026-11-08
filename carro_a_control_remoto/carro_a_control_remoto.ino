#include "MotorDriver.h"
#include "GamepadController.h"
#include "LineFollower.h"

// Configuración del controlador PID para el seguidor de línea
PIDConfig piTune = {
  .Kp = 0.65f,
  .Ki = 0.000f,  // En curvas de 90° se mantiene en 0 para evitar sobreimpulso
  .Kd = 3.250f,  // Término derivativo: anticipa esquinas y amortigua oscilaciones
  .baseSpeed = 65,
  .topSpeed = 255,
  .maxControl = 200,
  .integralLimit = 2000.0f
};

bool autonomousMode = false; // false = Control Remoto Manual (Azul), true = Seguidor de Línea (Verde)
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(1500); // Pausa inicial tras encender para soltar el carro sobre la línea

  Serial.println("=== INICIALIZANDO SISTEMA INTEGRADO ===");

  // 1. Inicialización de motores y sensores
  initMotorDriver();
  initLineFollower();

  // 2. Autocalibración automática: el carro barre izquierda/derecha y se centra en la línea
  Serial.println(">> AUTOCALIBRANDO SENSORES: El carro girara sobre la linea...");
  autoCalibrateLineSensor();
  Serial.println(">> Autocalibracion y centrado listos.");

  // 3. Inicialización del mando DualShock 4
  initGamepad();
}

void loop() {
  GamepadInput pad = readGamepad();

  if (pad.connected) {
    // A. Alternar entre Modo Manual y Autónomo al presionar el botón PS
    if (pad.psButtonJustPressed) {
      autonomousMode = !autonomousMode;
      stopMotors();
      resetPI();

      if (autonomousMode) {
        Serial.println("\n>>> MODO AUTONOMO (SEGUIDOR DE LINEA) ACTIVADO <<<");
        setGamepadLed(0, 255, 0); // Verde = Autónomo
      } else {
        Serial.println("\n>>> MODO CONTROL REMOTO (MANUAL) ACTIVADO <<<");
        setGamepadLed(0, 0, 255); // Azul = Manual
      }
    }

    // B. Recalibrar sensores en cualquier momento presionando TRIÁNGULO u OPTIONS en modo manual
    if (!autonomousMode && pad.calibrateHeld) {
      stopMotors();
      Serial.println(">> Ejecutando autocalibracion desde el mando...");
      autoCalibrateLineSensor();
      Serial.println(">> Autocalibracion completada.");
      return;
    }

    // C. Ejecución según el modo activo
    if (autonomousMode) {
      // Modo Seguidor de Línea Autónomo
      LineCorrection corr = computeLineCorrection(piTune);
      setChassisSpeed(corr.speedLeft, corr.speedRight);

      if (millis() - lastPrint >= 100) {
        lastPrint = millis();
        Serial.printf("[AUTONOMO] Pos: %4d | Error: %5d | Izq: %4d | Der: %4d | Perdio: %s\n",
                      corr.position,
                      corr.error,
                      corr.speedLeft,
                      corr.speedRight,
                      corr.lineLost ? "SI" : "NO");
      }
      delay(10); // ~100 Hz para el lazo PID
    } else {
      // Modo Manual (Arcade Drive con DualShock 4)
      int speedLeft  = constrain(pad.throttle + pad.steering, -255, 255);
      int speedRight = constrain(pad.throttle - pad.steering, -255, 255);

      setChassisSpeed(speedLeft, speedRight);

      if (millis() - lastPrint >= 200) {
        lastPrint = millis();
        Serial.printf("[MANUAL]   Thr: %4d | Str: %4d | Izq: %4d | Der: %4d\n",
                      pad.throttle,
                      pad.steering,
                      speedLeft,
                      speedRight);
      }
      delay(20); // ~50 Hz para control manual fluido
    }
  } else {
    // Failsafe: Si el control se desconecta, frenar motores y volver a modo manual por seguridad
    autonomousMode = false;
    stopMotors();
    delay(100);
  }
}
