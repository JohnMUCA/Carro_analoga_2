#include "GamepadController.h"

// Callback al establecer enlace Bluetooth
static void onConnectCallback() {
  Serial.println("\n>>> Mando PS4 Conectado Exitosamente! <<<");
  PS4.setLed(0, 0, 255); // Azul fijo
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

GamepadInput readGamepad() {
  GamepadInput input = {0, 0, false, 0};

  if (!PS4.isConnected()) {
    return input;
  }

  input.connected = true;
  input.battery = PS4.Battery();

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