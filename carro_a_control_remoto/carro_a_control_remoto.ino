#include <PS4Controller.h>

// ==========================================
// CONFIGURACIÓN DE PINES (PUENTE H)
// ==========================================
const int in1 = 33;  // Motor izquierdo adelante
const int in2 = 32;  // Motor izquierdo atrás
const int in3 = 26;  // Motor derecho adelante
const int in4 = 25;  // Motor derecho atrás

// ==========================================
// CONFIGURACIÓN PWM (Core 2.0.17)
// ==========================================
const int frecuencia = 1000; // 1 kHz
const int resolucion = 8;    // 8 bits (0 a 255)

// En Core v2.x usamos canales de hardware (0 a 15)
const int canal_in1 = 0;
const int canal_in2 = 1;
const int canal_in3 = 2;
const int canal_in4 = 3;

// ==========================================
// DECLARACIÓN DE FUNCIONES
// ==========================================
void setMotores(int vel_izquierdo, int vel_derecho);
void detenerMotores();

void onConnect() {
  Serial.println("\n>>> Mando PS4 conectado <<<");
  PS4.setLed(0, 0, 255); // Azul fijo en reposo
  PS4.sendToController();
}

void onDisconnect() {
  Serial.println("\n>>> Mando PS4 desconectado. Deteniendo carro... <<<");
  detenerMotores();
}

void setup() {
  Serial.begin(115200);

  // 1. Configurar canales PWM de hardware
  ledcSetup(canal_in1, frecuencia, resolucion);
  ledcSetup(canal_in2, frecuencia, resolucion);
  ledcSetup(canal_in3, frecuencia, resolucion);
  ledcSetup(canal_in4, frecuencia, resolucion);

  // 2. Asociar canales a los pines físicos
  ledcAttachPin(in1, canal_in1);
  ledcAttachPin(in2, canal_in2);
  ledcAttachPin(in3, canal_in3);
  ledcAttachPin(in4, canal_in4);

  // 3. Paro inicial de seguridad
  detenerMotores();

  // 4. Inicializar Bluetooth PS4
  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisconnect);

  // Si configuraste una MAC explícita, colócala dentro de las comillas:
  // PS4.begin("XX:XX:XX:XX:XX:XX");
  PS4.begin();

  Serial.println("Sistema listo. Enciende el control PS4.");
}

void loop() {
  if (PS4.isConnected()) {
    // ----------------------------------------------------
    // 1. Lectura de mandos
    // ----------------------------------------------------
    int r2 = PS4.R2Value();      // 0 a 255 (Adelante)
    int l2 = PS4.L2Value();      // 0 a 255 (Atrás)
    int stickX = PS4.LStickX();  // -127 (izq) a 127 (der)

    // Avance neto: si presionas R2 avanza, si presionas L2 retrocede
    int avance = r2 - l2; 

    // Zona muerta en el joystick para evitar desvíos parásitos
    int giro = stickX;
    if (abs(giro) < 15) {
      giro = 0;
    }

    // Escalamos el giro de [-127, 127] a [-200, 200]
    int correccionGiro = map(giro, -127, 127, -200, 200);

    // ----------------------------------------------------
    // 2. Mezcla para tracción diferencial
    // ----------------------------------------------------
    int velIzq = avance + correccionGiro;
    int velDer = avance - correccionGiro;

    // Saturación de seguridad dentro del rango PWM [-255, 255]
    velIzq = constrain(velIzq, -255, 255);
    velDer = constrain(velDer, -255, 255);

    // ----------------------------------------------------
    // 3. Aplicar potencia a las ruedas
    // ----------------------------------------------------
    setMotores(velIzq, velDer);

    // ----------------------------------------------------
    // 4. Indicador visual LED en el mando
    // ----------------------------------------------------
    if (avance > 20) {
      PS4.setLed(0, 255, 0); // Verde acelerando
    } else if (avance < -20) {
      PS4.setLed(255, 0, 0); // Rojo retrocediendo
    } else {
      PS4.setLed(0, 0, 255); // Azul reposo
    }
    PS4.sendToController();

    delay(20); // Bucle estable a 50 Hz
  } else {
    // Si se pierde la señal, frenar en seco de inmediato
    detenerMotores();
    delay(200);
  }
}

// ==========================================
// FUNCIONES DE CONTROL DE MOTORES
// ==========================================
void setMotores(int vel_izquierdo, int vel_derecho) {
  // Motor Izquierdo (IN1, IN2)
  if (vel_izquierdo > 0) {
    ledcWrite(canal_in2, 0);
    ledcWrite(canal_in1, vel_izquierdo);
  } else if (vel_izquierdo < 0) {
    ledcWrite(canal_in1, 0);
    ledcWrite(canal_in2, abs(vel_izquierdo));
  } else {
    ledcWrite(canal_in1, 0);
    ledcWrite(canal_in2, 0);
  }

  // Motor Derecho (IN3, IN4)
  if (vel_derecho > 0) {
    ledcWrite(canal_in4, 0);
    ledcWrite(canal_in3, vel_derecho);
  } else if (vel_derecho < 0) {
    ledcWrite(canal_in3, 0);
    ledcWrite(canal_in4, abs(vel_derecho));
  } else {
    ledcWrite(canal_in3, 0);
    ledcWrite(canal_in4, 0);
  }
}

void detenerMotores() {
  ledcWrite(canal_in1, 0);
  ledcWrite(canal_in2, 0);
  ledcWrite(canal_in3, 0);
  ledcWrite(canal_in4, 0);
}