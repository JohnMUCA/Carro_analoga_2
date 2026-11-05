#include "GamepadController.h"

static bool lastPsButtonState = false;

// Callback al establecer enlace Bluetooth
static void onConnectCallback() {
  Serial.println("\n>>> Mando PS4 Conectado Exitosamente! <<<");
  PS4.setLed(0, 0, 255); // Azul fijo (Modo Manual por defecto)
  PS4.sendToController();
}

// Callback al perder enlace Bluetooth
static void onDisconnectCallback() {
  Serial.println("\n>>> Mando PS4 Desconectado <<<");
}

void initGamepad() {
  Serial.println("Iniciando servicio Bluetooth PS4...");
  PS4.attachOnConnect(onConnectCallback);
  PS4.attachOnDisconnect(onDisconnectCallback);
  PS4.begin((char*)PS4_MAC_ADDRESS);
  Serial.println("Esperando enlace. Enciende el control con el boton PS...");
}

void setGamepadLed(uint8_t r, uint8_t g, uint8_t b) {
  if (PS4.isConnected()) {
    PS4.setLed(r, g, b);
    PS4.sendToController();
  }
}

GamepadInput readGamepad() {
  GamepadInput input = {0, 0, false, 0, false, false};

  if (!PS4.isConnected()) {
    lastPsButtonState = false;
    return input;
  }

  input.connected = true;
  input.battery = PS4.Battery();

  // Detección de flanco de subida en el botón PS (evita rebotes al cambiar de modo)
  bool currentPsButton = PS4.PSButton();
  if (currentPsButton && !lastPsButtonState) {
    input.psButtonJustPressed = true;
  }
  lastPsButtonState = currentPsButton;

  // Botón Options o Triángulo mantenido para calibrar sensores desde el mando
  input.calibrateHeld = PS4.Options() || PS4.Triangle();

  // Lectura de gatillos (0 a 255)
  int r2 = PS4.R2Value();
  int l2 = PS4.L2Value();

  // Acelerador longitudinal: R2 avanza (+), L2 retrocede (-)
  input.throttle = r2 - l2;

  // Lectura de dirección (-127 a 127) y aplicación de zona muerta
  int rawX = PS4.LStickX();
  if (abs(rawX) < STICK_DEADZONE) {
    rawX = 0;
  }

  // Escalar giro de [-127, 127] al rango PWM [-255, 255]
  input.steering = map(rawX, -127, 127, -255, 255);

  return input;
}
