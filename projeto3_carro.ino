
/*
 * Projeto 3 - EA 076 Carrinho que desvia de obstaculos
 * Caio Cesar Porto RA 101720
 * Daniel           RA 155090
 */

 #include <stdio.h>
 #include <TimerOne.h> 


/*Pinos relativos ao motor 1 */
int mot1_PWM = 5;
int mot1_front = 3;
int mot1_rear = 4;

/*Pinos relativos ao motor 2 */
int mot2_PWM= 6;
int mot2_front= 7;
int mot2_rear= 8;

/*Sensores*/
int sensorPin= A3;
int sens_value;

/*Flags pada controle do estado*/

int front_flag = 1 ;
int turn_flag = 0 ;
int count;

/* Funcao recebe como parametro o motor que deve ser ligado, 
direcao e velocidade e seta os pinos de saida */

void motor_on(int motor, boolean reverse,int speed){



switch(motor){

  case 1:
    analogWrite(mot1_PWM, speed);

    if(reverse == true){
        digitalWrite(mot1_front, HIGH);
        digitalWrite(mot1_rear, LOW);
    }

    else{
        digitalWrite(mot1_front, LOW);
        digitalWrite(mot1_rear, HIGH);
      
      }
    break;
    
  case 2:
    analogWrite(mot2_PWM, speed);

    if(reverse == true){
        digitalWrite(mot2_front, HIGH);
        digitalWrite(mot2_rear, LOW);
    }

    else{
        digitalWrite(mot2_front, LOW);
        digitalWrite(mot2_rear, HIGH);
    }
    break;
  
  }

  }


/* Funcao desliga ambos os motores*/

void motor_off(){
analogWrite(mot1_PWM, 0);
analogWrite(mot2_PWM, 0);

  
}

void setup() {

Serial.begin (9600);

pinMode(mot1_PWM, OUTPUT);
pinMode(mot1_front, OUTPUT);
pinMode(mot1_rear, OUTPUT);

pinMode(mot1_PWM, OUTPUT);
pinMode(mot1_front, OUTPUT);
pinMode(mot1_rear, OUTPUT);

pinMode(sensorPin,INPUT);

Timer1.initialize(10000); // Interrupcao a cada 1ms
Timer1.attachInterrupt(ISR_timer); // Associa a interrupcao periodica a funcao ISR_timer

}



/* A cada interrupcao o sistema verifica se o sensor esta ativo e em caso positivo muda o estado atraves das flags. Para voltar ao 
estado inicial o sensor deve ficar inativo durante 1000 contagens para evitar interferencia*/

void ISR_timer(){

sens_value = analogRead(sensorPin);
Serial.println(analogRead(sensorPin));

if(analogRead(sensorPin) <= 1000){

digitalWrite(13, HIGH); 

front_flag = 0;
turn_flag = 1;
count =0;  
}

if((analogRead(sensorPin) >= 1000)&& (count >= 50)){

digitalWrite(13, LOW); 
front_flag = 1;
turn_flag = 0;
}

count++;
  
}

void loop() {


if(front_flag == 1){

motor_on(1, true, 255);
motor_on(2, false, 255);

}


if(turn_flag == 1){

motor_on(1, false, 255);
motor_on(2, false, 255);

}


}
