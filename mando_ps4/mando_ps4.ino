#include "GamepadController.h"

// Temporizador no bloqueante para imprimir en consola cada 100 ms
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 100;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("--- PRUEBA MODULAR DEL MANDO PS4 ---");
  initGamepad();
}

void loop() {
  // Lectura del módulo del control
  GamepadInput pad = readGamepad();

  if (pad.connected) {
    // Imprimir datos a intervalos fijos sin usar delay() pesado
    if (millis() - lastPrintTime >= printInterval) {
      lastPrintTime = millis();

      Serial.printf("Throttle (R2 - L2): %4d | Steering (Stick X): %4d | Batería: %3d%%\n",
                    pad.throttle, pad.steering, pad.battery);
    }
  } else {
    // Si no está conectado, avisa periódicamente
    if (millis() - lastPrintTime >= 1000) {
      lastPrintTime = millis();
      Serial.println("Esperando conexión del DualShock 4...");
    }
  }
}