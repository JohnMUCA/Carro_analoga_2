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

}

void loop() {
  
  for(int duty = 0; duty <= 255; duty += 5)
  {
    ledcWrite(in1, duty);
    ledcWrite(in3, duty);
    ledcWrite(in2, 0);
    ledcWrite(in4, 0);
    delay(100);
  }
  delay(1000);

  for(int duty = 0; duty <= 255; duty += 5)
  {
    ledcWrite(in1, 0);
    ledcWrite(in3, 0);
    delay(20);
    ledcWrite(in2, duty);
    ledcWrite(in4, duty);
    delay(100);
  }
  delay(1000);

}
