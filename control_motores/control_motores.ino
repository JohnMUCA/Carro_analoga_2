//Implementación pwm para motores

const int in1 = 33;  // In1 forward motor 1 (izquierdo)
const int in2 = 32;  // In2 backward motor 1 (izquierdo)
const int in3 = 26;  // In3 forward motor 2 (derecho)
const int in4 = 25;  // In4 backward motor 2 (derecho)

const int frecuencia = 1000;  //1KHz  fecuencia pwm motores
const int resolucion = 8; //8 bits  0 a 255  


void setup() {
  
  // Se inicializan los pines para el pwm de los motores
  ledcAttach(in1, frecuencia, resolucion);
  ledcAttach(in2, frecuencia, resolucion);
  ledcAttach(in3, frecuencia, resolucion);
  ledcAttach(in4, frecuencia, resolucion);

  // Asegurar paro inicial
  setMotores(0, 0);
  delay(1000);
  Serial.println("Iniciando prueba de traccion...");

}

void loop() {
  
  // 1. Adelante al ~70% de potencia
  Serial.println(">> Adelante");
  setMotores(180, 180);
  delay(1500);

  setMotores(0, 0);
  delay(500); // Pausa de inercia

  // 2. Atras al ~70% de potencia
  Serial.println(">> Atras");
  setMotores(-180, -180);
  delay(1500);

  setMotores(0, 0);
  delay(500);

  // 3. Giro a la derecha en el sitio (izq avanza, der retrocede)
  Serial.println(">> Giro Derecha");
  setMotores(180, -180);
  delay(1500);

  setMotores(0, 0);
  delay(500);

  // 4. Giro a la izquierda en el sitio (izq retrocede, der avanza)
  Serial.println(">> Giro Izquierda");
  setMotores(-180, 180);
  delay(1500);

  // Pausa larga antes de reiniciar ciclo
  Serial.println(">> Fin ciclo. Esperando 3s...\n");
  setMotores(0, 0);
  delay(3000);
  

}


void setMotores(int vel_izquierdo, int vel_derecho){

// Control motor izquierdo IN1, IN2
  if(vel_izquierdo > 0){
    ledcWrite(in2, 0);
    ledcWrite(in1, vel_izquierdo);
  } else if (vel_izquierdo < 0){
    ledcWrite(in1, 0);
    ledcWrite(in2, abs(vel_izquierdo));
  } else {
    ledcWrite(in1, 0);
    ledcWrite(in2, 0);
  }


// Control motor derecho IN3, IN4
  if(vel_derecho > 0){
    ledcWrite(in4, 0);
    ledcWrite(in3, vel_derecho);
  } else if (vel_derecho < 0){
    ledcWrite(in3, 0);
    ledcWrite(in4, abs(vel_derecho));
  } else {
    ledcWrite(in3, 0);
    ledcWrite(in4, 0);
  }

}
