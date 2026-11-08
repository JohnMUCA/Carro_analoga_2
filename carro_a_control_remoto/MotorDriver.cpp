#include "MotorDriver.h"

// Vincula un par de canales PWM con sus pines GPIO
static void configureChannelPair(uint8_t ch1, int pin1, uint8_t ch2, int pin2) {
  ledcSetup(ch1, PWM_FREQ, PWM_RES);
  ledcAttachPin(pin1, ch1);

  ledcSetup(ch2, PWM_FREQ, PWM_RES);
  ledcAttachPin(pin2, ch2);
}

// Aplica velocidad (-255 a 255) a un par específico de canales
static void setMotorPWM(uint8_t chForward, uint8_t chBackward, int speed) {
  speed = constrain(speed, -255, 255);

  if (speed > 0) {
    ledcWrite(chBackward, 0);
    ledcWrite(chForward, speed);
  } else if (speed < 0) {
    ledcWrite(chForward, 0);
    ledcWrite(chBackward, abs(speed));
  } else {
    ledcWrite(chForward, 0);
    ledcWrite(chBackward, 0);
  }
}

void initMotorDriver() {
  configureChannelPair(CH_FL_IN3, PIN_M_FL_IN3, CH_FL_IN4, PIN_M_FL_IN4);
  configureChannelPair(CH_RL_IN1, PIN_M_RL_IN1, CH_RL_IN2, PIN_M_RL_IN2);
  configureChannelPair(CH_FR_IN1, PIN_M_FR_IN1, CH_FR_IN2, PIN_M_FR_IN2);
  configureChannelPair(CH_RR_IN3, PIN_M_RR_IN3, CH_RR_IN4, PIN_M_RR_IN4);

  stopMotors();
}

// Control diferencial: sincroniza los 2 motores izquierdos y los 2 derechos
void setChassisSpeed(int speedLeft, int speedRight) {
  // Lado izquierdo
  setMotorPWM(CH_FL_IN3, CH_FL_IN4, speedLeft);
  setMotorPWM(CH_RL_IN1, CH_RL_IN2, speedLeft);

  // Lado derecho
  setMotorPWM(CH_FR_IN1, CH_FR_IN2, speedRight);
  setMotorPWM(CH_RR_IN3, CH_RR_IN4, speedRight);
}

void stopMotors() {
  setChassisSpeed(0, 0);
}
