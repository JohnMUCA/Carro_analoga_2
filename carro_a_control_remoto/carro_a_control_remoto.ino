#include "MotorDriver.h"
#include "GamepadController.h"
#include "LineFollower.h"

// Configuración inicial del controlador PI para el seguidor de línea
PIDConfig piTune = {
  .Kp = 0.045f,
  .Ki = 0.005f,
  .baseSpeed = 160,
  .maxControl = 200,
  .integralLimit = 2000.0f
};

bool autonomousMode = false; // false = Control Remoto (Azul), true = Seguidor de Línea (Verde)
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 1. Inicialización de motores y sensores
  initMotorDriver();
  initLineFollower();

  // 2. Calibración inicial de 6 segundos (igual que en seguidor_linea.ino)
  // También puedes recalibrar en cualquier momento manteniendo presionado TRIÁNGULO u OPTIONS en el mando PS4
  Serial.println("=== INICIALIZANDO SISTEMA INTEGRADO ===");
  Serial.println(">> CALIBRANDO SENSORES (6 s): Mueve el sensor sobre la linea y el fondo...");
  unsigned long calibStart = millis();
  while (millis() - calibStart < 6000) {
    calibrateLineSensor();
    delay(10);
  }
  Serial.println(">> Calibracion inicial lista.");

  // 3. Inicialización del mando PS4
  initGamepad();
}

void loop() {
  GamepadInput pad = readGamepad();

  if (pad.connected) {
    // A. Cambiar entre Modo Control Remoto y Modo Autónomo al presionar el botón PS
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

    // B. Si se mantiene presionado TRIÁNGULO u OPTIONS en modo manual, recalibra sensores en vivo
    if (!autonomousMode && pad.calibrateHeld) {
      stopMotors();
      calibrateLineSensor();
      if (millis() - lastPrint >= 200) {
        lastPrint = millis();
        Serial.println(">> Calibrando sensores desde el mando (Mueve el carro sobre blanco y negro)...");
      }
      delay(10);
      return;
    }

    // C. Ejecución según el modo activo
    if (autonomousMode) {
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
      delay(10); // ~100 Hz para el lazo PI
    } else {
      // Modo Manual (Arcade Drive)
      int speedLeft  = constrain(pad.throttle + pad.steering, -255, 255);
      int speedRight = constrain(pad.throttle - pad.steering, -255, 255);

      setChassisSpeed(speedLeft, speedRight);

      // En modo manual también leemos la línea cada 200 ms solo para verificar en el Monitor Serie que el Bluetooth no bloquee el sensor
      if (millis() - lastPrint >= 200) {
        lastPrint = millis();
        LineCorrection corr = computeLineCorrection(piTune);
        Serial.printf("[MANUAL]   Pos: %4d | Error: %5d | Thr: %4d | Str: %4d | Perdio: %s\n",
                      corr.position,
                      corr.error,
                      pad.throttle,
                      pad.steering,
                      corr.lineLost ? "SI" : "NO");
      }
      delay(20); // ~50 Hz para control manual
    }
  } else {
    // Failsafe: Si el control se desconecta, frenar motores y volver a modo manual por seguridad
    autonomousMode = false;
    stopMotors();
    delay(100);
  }
}
