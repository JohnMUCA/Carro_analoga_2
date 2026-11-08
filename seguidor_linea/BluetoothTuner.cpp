#include "BluetoothTuner.h"

static BluetoothSerial SerialBT;

// Envía texto simultáneamente al Monitor Serie USB y a la Terminal Bluetooth
static void broadcastPrintln(const String &msg) {
  Serial.println(msg);
  if (SerialBT.hasClient()) {
    SerialBT.println(msg);
  }
}

void initBluetoothTuner() {
  SerialBT.begin(BT_TUNER_NAME);
  Serial.println("\n=== SINTONIZADOR PID BLUETOOTH INICIADO ===");
  Serial.printf(">> Dispositivo Bluetooth: '%s'\n", BT_TUNER_NAME);
  Serial.println(">> Conectate desde 'Serial Bluetooth Terminal' en Android/PC.");
  Serial.println(">> Escribe '?' o 'help' para ver los comandos disponibles.\n");
}

void printTunerStatus(const PIDConfig &pid, const TunerState &state) {
  char buf[340];
  snprintf(buf, sizeof(buf),
           "\n--- ESTADO ACTUAL DEL CONTROLADOR PID ---\n"
           "  Motores   : %s\n"
           "  Kp        : %.4f\n"
           "  Ki        : %.4f\n"
           "  Kd        : %.4f\n"
           "  baseSpeed : %d\n"
           "  topSpeed  : %d\n"
           "  maxControl: %d\n"
           "-----------------------------------------\n"
           "Comandos:\n"
           "  kp=0.65   -> Cambiar Kp\n"
           "  kd=3.25   -> Cambiar Kd\n"
           "  ki=0.000  -> Cambiar Ki\n"
           "  base=65   -> Cambiar velocidad base\n"
           "  top=255   -> Cambiar velocidad tope\n"
           "  max=200   -> Cambiar limite maxControl\n"
           "  vel=80    -> Escalar proporcionalmente ganancias y velocidades\n"
           "  go  / 1   -> Arrancar motores en pista\n"
           "  stop / 0  -> Frenar motores inmediatamente\n"
           "  cal       -> Autocalibrar girando el carro sobre la linea\n"
           "  log       -> Pausar/Reanudar telemetria en pantalla\n"
           "  ?         -> Ver este resumen\n",
           state.motorsRunning ? "EN MARCHA (GO)" : "DETENIDOS (STOP)",
           pid.Kp, pid.Ki, pid.Kd, pid.baseSpeed, pid.topSpeed, pid.maxControl);
  broadcastPrintln(String(buf));
}

// Escala proporcionalmente velocidades y ganancias PID por un factor dado
static void applyProportionalScale(PIDConfig &pid, float factor) {
  if (factor <= 0.0f) return;

  pid.baseSpeed  = constrain((int)roundf(pid.baseSpeed * factor), 0, 255);
  pid.topSpeed   = constrain((int)roundf(pid.topSpeed * factor), 1, 255);
  pid.maxControl = constrain((int)roundf(pid.maxControl * factor), 1, 255);
  pid.Kp        *= factor;
  pid.Ki        *= factor;
  pid.Kd        *= factor;
  resetPI();
}

static void processCommand(String cmd, PIDConfig &pid, TunerState &state) {
  cmd.trim();
  if (cmd.length() == 0) return;

  String lower = cmd;
  lower.toLowerCase();

  // 1. Comandos de marcha, parada y utilidades
  if (lower == "stop" || lower == "s" || lower == "0") {
    state.motorsRunning = false;
    stopMotors();
    resetPI();
    broadcastPrintln(">> [STOP] Motores detenidos.");
    return;
  }

  if (lower == "go" || lower == "run" || lower == "1") {
    resetPI();
    state.motorsRunning = true;
    broadcastPrintln(">> [GO] Modo seguidor en marcha!");
    return;
  }

  if (lower == "log") {
    state.telemetryActive = !state.telemetryActive;
    broadcastPrintln(String(">> Telemetria en vivo: ") + (state.telemetryActive ? "ACTIVADA" : "PAUSADA"));
    return;
  }

  if (lower == "?" || lower == "help" || lower == "status") {
    printTunerStatus(pid, state);
    return;
  }

  if (lower == "cal") {
    state.motorsRunning = false;
    stopMotors();
    broadcastPrintln(">> [CAL] Iniciando autocalibracion (el carro girara sobre la linea)...");
    delay(400);
    autoCalibrateLineSensor();
    broadcastPrintln(">> [CAL] Autocalibracion y centrado listos. Envia 'go' para arrancar.");
    return;
  }

  // 2. Comandos con valor (admite 'kp=0.65' o 'kp 0.65')
  int sepIndex = lower.indexOf('=');
  if (sepIndex < 0) {
    sepIndex = lower.indexOf(' ');
  }

  if (sepIndex > 0) {
    String key = lower.substring(0, sepIndex);
    String valStr = lower.substring(sepIndex + 1);
    key.trim();
    valStr.trim();
    float val = valStr.toFloat();

    if (key == "kp") {
      pid.Kp = val;
      resetPI();
      broadcastPrintln(">> Nuevo Kp = " + String(pid.Kp, 4));
    } else if (key == "ki") {
      pid.Ki = val;
      resetPI();
      broadcastPrintln(">> Nuevo Ki = " + String(pid.Ki, 4));
    } else if (key == "kd") {
      pid.Kd = val;
      resetPI();
      broadcastPrintln(">> Nuevo Kd = " + String(pid.Kd, 4));
    } else if (key == "base") {
      pid.baseSpeed = constrain((int)val, 0, 255);
      broadcastPrintln(">> Nuevo baseSpeed = " + String(pid.baseSpeed));
    } else if (key == "top") {
      pid.topSpeed = constrain((int)val, 1, 255);
      broadcastPrintln(">> Nuevo topSpeed = " + String(pid.topSpeed));
    } else if (key == "max") {
      pid.maxControl = constrain((int)val, 1, 255);
      broadcastPrintln(">> Nuevo maxControl = " + String(pid.maxControl));
    } else if (key == "vel") {
      if (pid.baseSpeed > 0 && val > 0) {
        float factor = val / (float)pid.baseSpeed;
        applyProportionalScale(pid, factor);
        broadcastPrintln(">> Escalado proporcional aplicado (factor x" + String(factor, 2) + ")");
        printTunerStatus(pid, state);
      } else {
        broadcastPrintln(">> Error: baseSpeed actual debe ser > 0 para escalar con 'vel='.");
      }
    } else {
      broadcastPrintln(">> Comando desconocido: '" + cmd + "'. Escribe '?' para ayuda.");
    }
    return;
  }

  broadcastPrintln(">> Comando no reconocido: '" + cmd + "'. Escribe '?' para ver opciones.");
}

void updateBluetoothTuner(PIDConfig &pid, TunerState &state) {
  static String btBuffer = "";
  static String usbBuffer = "";

  // Leer desde Bluetooth Serial (celular/PC)
  while (SerialBT.available()) {
    char c = (char)SerialBT.read();
    if (c == '\n' || c == '\r') {
      if (btBuffer.length() > 0) {
        processCommand(btBuffer, pid, state);
        btBuffer = "";
      }
    } else {
      btBuffer += c;
    }
  }

  // Leer desde Monitor Serie USB
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (usbBuffer.length() > 0) {
        processCommand(usbBuffer, pid, state);
        usbBuffer = "";
      }
    } else {
      usbBuffer += c;
    }
  }
}

void sendTunerTelemetry(const PIDConfig &pid, const LineCorrection &corr) {
  char line[140];
  snprintf(line, sizeof(line),
           "[TUNER] Pos:%4d | Err:%5d | Izq:%4d | Der:%4d | Kp:%.3f Kd:%.3f Ki:%.4f | Base:%d",
           corr.position, corr.error, corr.speedLeft, corr.speedRight,
           pid.Kp, pid.Kd, pid.Ki, pid.baseSpeed);
  broadcastPrintln(String(line));
}
