#include "MotorDriver.h"
#include "GamepadController.h"
#include "LineFollower.h"
#include "BluetoothTuner.h"

// ============================================================================
// SELECTOR DE MODO DE OPERACIÓN:
//   true  -> Modo Sintonización PID por Bluetooth Serial ("Carro_Seguidor_PI")
//            Conéctate desde el celular/PC (app Serial Bluetooth Terminal)
//            para cambiar Kp, Kd, Ki, baseSpeed y topSpeed en vivo en la pista.
//   false -> Modo Integrado con Mando PS4 (Azul = Manual, Verde = Seguidor)
// ============================================================================
const bool USE_BT_TUNER = true;

// Configuración inicial del controlador PID para el seguidor de línea
PIDConfig piTune = {
  .Kp = 0.65f,
  .Ki = 0.000f,  // En curvas de 90° conviene iniciar con Ki = 0 para evitar sobreimpulso
  .Kd = 3.250f,  // Término derivativo: anticipa esquinas de 90° y amortigua zig-zag
  .baseSpeed = 65,
  .topSpeed = 255,
  .maxControl = 200,
  .integralLimit = 2000.0f
};

// Estado para el modo de sintonización Bluetooth
TunerState tunerState = {
  .motorsRunning = false,  // Inicia detenido por seguridad; envía 'go' o '1' para arrancar
  .telemetryActive = true
};

// Estado para el modo con mando PS4
bool autonomousMode = false; // false = Control Remoto (Azul), true = Seguidor de Línea (Verde)
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(1500); // Pausa inicial tras encender para soltar el carro sobre la línea

  // 1. Inicialización de motores y sensores
  initMotorDriver();
  initLineFollower();

  // 2. Autocalibración automática: el carro barre izquierda/derecha y se centra en la línea
  Serial.println("=== INICIALIZANDO SISTEMA ===");
  Serial.println(">> AUTOCALIBRANDO SENSORES: El carro girara sobre la linea...");
  autoCalibrateLineSensor();
  Serial.println(">> Autocalibracion y centrado finalizados.");

  // 3. Inicializar Bluetooth según el modo elegido
  if (USE_BT_TUNER) {
    initBluetoothTuner();
    printTunerStatus(piTune, tunerState);
  } else {
    initGamepad();
  }
}

void loop() {
  // --------------------------------------------------------------------------
  // MODO 1: SINTONIZACIÓN EN TIEMPO REAL POR BLUETOOTH SERIAL / MONITOR SERIE
  // --------------------------------------------------------------------------
  if (USE_BT_TUNER) {
    updateBluetoothTuner(piTune, tunerState);

    LineCorrection corr = computeLineCorrection(piTune);

    if (tunerState.motorsRunning) {
      setChassisSpeed(corr.speedLeft, corr.speedRight);
    } else {
      stopMotors();
    }

    if (tunerState.telemetryActive && (millis() - lastPrint >= 150)) {
      lastPrint = millis();
      sendTunerTelemetry(piTune, corr);
    }

    delay(10); // ~100 Hz para el lazo PID
    return;
  }

  // --------------------------------------------------------------------------
  // MODO 2: OPERACIÓN CON MANDO PS4 (MANUAL / AUTÓNOMO)
  // --------------------------------------------------------------------------
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

    // B. Si se presiona TRIÁNGULO u OPTIONS en modo manual, ejecuta la autocalibración
    if (!autonomousMode && pad.calibrateHeld) {
      Serial.println(">> Ejecutando autocalibracion automatica desde el mando...");
      autoCalibrateLineSensor();
      Serial.println(">> Autocalibracion completada.");
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
      delay(10); // ~100 Hz para el lazo PID
    } else {
      // Modo Manual (Arcade Drive)
      int speedLeft  = constrain(pad.throttle + pad.steering, -255, 255);
      int speedRight = constrain(pad.throttle - pad.steering, -255, 255);

      setChassisSpeed(speedLeft, speedRight);

      // En modo manual también leemos la línea cada 200 ms para verificar lecturas en Monitor Serie
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
