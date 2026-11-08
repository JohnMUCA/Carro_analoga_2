#ifndef GAMEPAD_CONTROLLER_H
#define GAMEPAD_CONTROLLER_H

#include <Arduino.h>
#include <PS4Controller.h>

// MAC Bluetooth configurada para el enlace del mando
const char PS4_MAC_ADDRESS[] = "EC:C9:FF:FD:F8:FA";

// Umbral de zona muerta para evitar oscilaciones por holgura mecánica del stick
const int STICK_DEADZONE = 12;

// Estructura para entregar datos limpios al loop principal
struct GamepadInput {
  int throttle;             // Consigna longitudinal (-255 reversa a +255 avance)
  int steering;             // Consigna de giro (-255 izquierda a +255 derecha)
  bool connected;
  bool psButtonJustPressed; // True solo en el instante en que se presiona el botón PS
  bool calibrateHeld;       // True mientras se mantiene presionado Options/Triángulo para calibrar
};

// Funciones públicas del módulo
void initGamepad();
GamepadInput readGamepad();
void setGamepadLed(uint8_t r, uint8_t g, uint8_t b);

#endif
