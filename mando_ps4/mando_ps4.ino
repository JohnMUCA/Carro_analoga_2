#include <PS4Controller.h>

void onConnect() {
  Serial.println("\n>>> Mando PS4 Conectado Exitosamente! <<<");
  // Poner el LED del mando en azul fijo al conectar
  PS4.setLed(0, 0, 255);
  PS4.sendToController();
}

void onDisConnect() {
  Serial.println("\n>>> Mando PS4 Desconectado <<<");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Iniciando servicio Bluetooth PS4...");

  // Callbacks para eventos de conexion
  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisConnect);

  // Iniciar el stack Bluetooth. 
  // Al dejarlo vacio, toma la MAC nativa de la ESP32
  PS4.begin("EC:C9:FF:FD:F8:FA");

  Serial.println("Esperando enlace. Enciende el control con el boton PS...");
}

void loop() {
  if (PS4.isConnected()) {
    // Lectura de los controles que usaremos para el carro
    int avance  = PS4.R2Value();  // Gatillo R2: 0 a 255
    int reversa = PS4.L2Value();  // Gatillo L2: 0 a 255
    int giroX   = PS4.LStickX();  // Palanca izquierda X: -127 a 127

    // Imprimir telemetría
    Serial.printf("R2 (Adelante): %3d | L2 (Reversa): %3d | Stick X (Giro): %4d | Bateria: %d%%\n",
                  avance, reversa, giroX, PS4.Battery());

    delay(50); // Muestreo fluido para la consola
  } else {
    delay(500);
  }
}