
/*
 * Projeto 4 - EA 076 Osciloscopio de interface serial
 * Caio Cesar Porto RA 101720
 * Daniel           RA 155090
 */





#include <TimerOne.h>               //Biblioteca utilizada para temporização

int read_cont = 0 ;
int sensorValue;
int amostr = 14 ;                   //Esta variavel define a taxa de amostragem do sinal de entrada
void setup() {
  
Serial.begin(9600);                //inicializaçao da serial
Timer1.initialize(10000);          // Interrupcao a cada 1ms
Timer1.attachInterrupt(ISR_timer); // Associa a interrupcao periodica a funcao ISR_timer


}


/*
 * A cada periodo de tempo que é definido pela valor da variavel  read_cont é realizada a leitura do valor na entrada analogica e este valor é  escrito na porta serial 
 */

void ISR_timer() { 
    
        read_cont++;
      
    }


void loop() {

analogWrite(5, 100);            //sinal de referencia PWM no pino 5 

if (read_cont >= amostr){            // Esta linha define o periodo de amostragem do sinal 

sensorValue = analogRead(A0)/205; //Leitura do valor de tensão no pino A0

Serial.println(sensorValue);
  
}

 
}
