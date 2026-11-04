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
  int throttle; // Consigna longitudinal (-255 reversa a +255 avance)
  int steering; // Consigna de giro (-255 izquierda a +255 derecha)
  bool connected;
  int battery;
};

// Funciones públicas del módulo
void initGamepad();
GamepadInput readGamepad();

#endif