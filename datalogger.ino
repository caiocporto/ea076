
#include <TimerOne.h>               //Biblioteca utilizada para temporização
#include <Wire.h>                   //Biblioteca utilizada para comunicacao com EEPROM externa via I2C
#include <stdio.h>
#include <Keypad.h>                 //Biblioteca para controle do teclado obtida em http://www.arduinoecia.com.br/2015/05/teclado-matricial-membrana-4x3-arduino.html

#define MAX_BUFFER_SIZE  25         // Definição do tamanho do Buffer 


/*STRUCT PARA CRIAÇÃO DOS BUFFERS*/

typedef struct {                    // Delcarao da Struct que sera utiizda para criacao do Buffer de dados recebidos
  char data[MAX_BUFFER_SIZE];
  unsigned int tam_buffer;
} serial_buffer;


/* Declaração das Variaveis globais*/

serial_buffer Buffer;             //Variavel Buffer do tipo serial_buffer para UART entrada 
serial_buffer Key_Buffer;         //Variavel Buffer para o Teclado 


char matriz_teclas[4][3] =         //Definição dos caracteres do teclado matricial
{
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte PinosLinhas[4] = {7, 8, 9, 10};  //Definicao dos pinos das linhas
byte PinosColunas[3] = {4, 5, 6};     //Definicao dos pinos das colunas

Keypad teclado = Keypad( makeKeymap(matriz_teclas), PinosLinhas, PinosColunas, 4, 3); 

int flag_check_command = 0;      //Variavel responsavel por sinalizar o final da recepção de um comando através da UART
int key_check_command = 0;       //Variavel responsavel por sinalizar o final da recepção de um comando através do teclado
int auto_check_command = 0;      //Variavel responsavel por sinalizar se medida automatica esta ativa

int sensorValue;                 //Variavel que contem os valores medidos através do LDR */
int memStatus;                   //Variavel que contem o indice da memoria (quantos bytes tem gravados)
int adress;                      //Variavel utilizada para endereço de memoria
int readData;
unsigned int auto_cont;         //Contador para temporizar a medição automatica periodica

/* FUNCOES RELATIVAS A UART */

    /* Compara 2 strings e caso sejam iguais retorna 1, do contrario retorna 0   */
      int str_cmp(char *s1, char *s2, int len) {
          int i;
        for (i=0; i<len; i++) {
          if (s1[i] != s2[i]) return 0;
          if (s1[i] == '\0') return 1;
       }
        return 1;
      }

      /* Adiciona caractere ao buffer */
    int buffer_add(char c_in) {
      if (Buffer.tam_buffer < MAX_BUFFER_SIZE) {
        Buffer.data[Buffer.tam_buffer++] = c_in;
        return 1;
      }
      return 0;
    } 

    /* Limpa buffer */
    
    void buffer_clean() {
      Buffer.tam_buffer = 0;
    }

/* FUNCOES RELATIVAS AO TECLADO */

      /* Adiciona caractere ao buffer do teclado*/
    int key_buffer_add(char c_in) {
      if (Key_Buffer.tam_buffer < MAX_BUFFER_SIZE) {
        Key_Buffer.data[Key_Buffer.tam_buffer++] = c_in;
        return 1;
      }
      return 0;
    } 

    /* Limpa buffer do teclado*/
    
    void key_buffer_clean() {
      Key_Buffer.tam_buffer = 0;
    }

    void check_key(){
          char tecla_pressionada = teclado.getKey();
        
            if (tecla_pressionada){
                   key_buffer_add(tecla_pressionada);
                   
                   if(tecla_pressionada == '*'){
                       Serial.println(tecla_pressionada);
                       key_check_command =1; 
                      
                              }
                  else Serial.print(tecla_pressionada);
                    
               
                    }
        }




/* FUNCOES RELATIVAS A ESCRITA NA EEPROM */

      /*Função de escrita recebe como parametros o id do chip de memoria o endereco de memoria a ser escrito e o dado e nao retorna nenhum resultado  */
      
      void memory_write(int deviceaddress, unsigned int eeaddress, byte data ) 
      {
        Wire.beginTransmission(deviceaddress);
        Wire.write((int)(eeaddress)); // MSB
        Wire.write(data);
        Wire.endTransmission();
        delay(5);
      }
      
      
      /*Função de escrita recebe como parametros o id do chip e o endereco de memoria a ser escrito e retorna o byte lido no endereço de memória */
      
      
      byte memory_read(int deviceaddress, unsigned int eeaddress ) 
      {
        byte rdata = 0xF;
       
        Wire.beginTransmission(deviceaddress);
        Wire.write((int)(eeaddress)); // MSB
        Wire.endTransmission();
        Wire.requestFrom(deviceaddress,1);
       
        if (Wire.available()) rdata = Wire.read();
       
        return rdata;
      }
      
 /* SETUP */

     void setup() {
      
      buffer_clean();
      flag_check_command = 0;
      
      /*inicialização da Serial e do I2C*/
      Wire.begin();
      Serial.begin(9600);
    
      /*Configuração de interrupcao*/
      Timer1.initialize(10000); // Interrupcao a cada 1ms
      Timer1.attachInterrupt(ISR_timer); // Associa a interrupcao periodica a funcao ISR_timer

      /*Linhas para teste de escrita e leitura na memória
      memory_write(0x50, 0x00, B1010);
      Serial.print(memory_read(0x50, 0x00), BIN);  */
      
    }

  /* ROTINA DE INTERRUPCAO POR TEMPO */
     void ISR_timer() { 
    
      sensorValue = analogRead(A0); //Leitura do valor de tensão no LDR
      check_key();
      auto_cont++;
      
    }

/* ROTINA RECEPÇÃO DE CARACTERES  */

    void serialEvent() {
      char c;
      while (Serial.available()) {
        c = Serial.read();
        if (c=='\n') {
          buffer_add('\0'); /* Se recebeu um fim de linha, coloca um terminador de string no buffer */
          flag_check_command = 1;
        } else {
         buffer_add(c);
        }
      }
    }
        

  /* ROTINA PINCIPAL - KERNEL EM TEMPO REAL 

  Foi criada uma flag para cada funcionalidade do sisteama:
  
   - flag_check_command: controla a funcao que trata os comandos recebidos via UART
   - key_check_command: controla a funcao que trata os comandos recebidos via teclado
   - auto_check_command: controla a funcao de mediçao automatica
   - flag_write controla funcao que escreve na serial o que esta no buffer de saida 
  Dessa forma nao há interferencia entre as funcionalidades e a sequencia de açoes do programa fica melhor organizada 
   */  
    
        void loop() {
  int x, y;
  char out_buffer[30];
  int flag_write = 0;


  /*COMANDOS UART*/
   
  if (flag_check_command == 1) {
    if (str_cmp(Buffer.data, "PING", 4) ) {
      sprintf(out_buffer, "PONG\n");
      flag_write = 1;
      buffer_clean();
    }

    if (str_cmp(Buffer.data, "ID", 2) ) {
      sprintf(out_buffer, "DATALOGGER - CAIO E DANIEL\n");
      flag_write = 1;
      buffer_clean();
    }

    if (str_cmp(Buffer.data, "MEASURE", 7) ) {

           sprintf(out_buffer, "LDR = %d\n", sensorValue);
      flag_write = 1;
      buffer_clean();
    }

 if (str_cmp(Buffer.data, "MEMSTATUS", 9) ) {


    memStatus = memory_read(0x50,0x00);

      sprintf(out_buffer, "N ELEMENTOS NA MEMORIA = %d\n", memStatus);
      flag_write = 1;
      buffer_clean();
    }

if (str_cmp(Buffer.data, "RESET", 5) ) {

     for(adress= 0; adress < 127 ; adress++ ){
                  memory_write(0x50, adress, 0x00);
        };
  
           sprintf(out_buffer, "MEMORIA APAGADA COM SUCESSO\n");
      flag_write = 1;
      buffer_clean();
    }

if (str_cmp(Buffer.data, "RECORD", 6) ) {

      int writeAdress;                 //Endereço de escrita na memoria   
      int writeData;                   //Dado a ser salvo na EEPROM
  
      
      memStatus = memory_read(0x50,0x00);
      writeAdress= memStatus + 1;
      writeData = sensorValue;
      memory_write(0x50, 0x00, writeAdress);
      memory_write(0x50, writeAdress, sensorValue);
    
 
           sprintf(out_buffer, "LDR = %d GRAVADO COM SUCESSO \n", writeData);
         flag_write = 1;
         buffer_clean();
        }

    if (str_cmp(Buffer.data, "GET ", 4) ) {

          /* O PARAMETRO PASSADO PELO COMANDO É ARMAZENADO NO VETOR vetPos e seguida esse valor é convertido para um inteiro para ser utilizado na funcao de leitura dos dados
           *  da memória. Sabendo a posição de memória é retornado no terminal o valor contido na posição de memoria utilizando o buffer de saida como nos outros comandos do terminal
           *  
           */
          int i;
          int j=0;
          char vetPos[4];
      
         
            for (i=4; i<8; i++){
              vetPos[j] = Buffer.data[i];
              j++;
                     }
                  
                  int memPos = atoi(vetPos);
                  
              sprintf(out_buffer, "POSICAO %d: %d\n",memPos,memory_read(0x50, memPos) );
          flag_write = 1;
          // buffer_clean();
       }
   
    flag_check_command = 0;
    buffer_clean();
  }
  

if (key_check_command == 1) {
    if (str_cmp(Key_Buffer.data, "#1*", 3) ) {
      
      for(int i=0; i<5; i++) {
            digitalWrite(13, HIGH);
            delay(250);
            digitalWrite(13, LOW);
            delay(250);
          }
      key_buffer_clean();
  
    }

    if (str_cmp(Key_Buffer.data, "#2*", 3) ) {

      int writeAdress;                 //Endereço de escrita na memoria   
      int writeData;                   //Dado a ser salvo na EEPROM
      
      memStatus = memory_read(0x50,0x00);
      writeAdress= memStatus + 1;
      writeData = sensorValue;
      memory_write(0x50, 0x00, writeAdress);
      memory_write(0x50, writeAdress, sensorValue);
    
 
           sprintf(out_buffer, "LDR = %d GRAVADO COM SUCESSO \n", writeData);
         flag_write = 1;
         key_buffer_clean();
    }




    
    if (str_cmp(Key_Buffer.data, "#3*", 3) ) {
      Serial.print("MEDICAO AUTOMATICA : LIGADO \n");
      key_buffer_clean();
      auto_check_command = 1;
    }
    if (str_cmp(Key_Buffer.data, "#4*", 3) ) {
      Serial.print("MEDICAO AUTOMATICA : DESLIGADO\n");
      key_buffer_clean();
      auto_check_command = 0;
    }

key_buffer_clean();
key_check_command = 0;


}
    
if (auto_check_command == 1) {
  
     if(auto_cont > 500){
      
            int writeAdress;                 //Endereço de escrita na memoria   
            int writeData;                   //Dado a ser salvo na EEPROM
            
            memStatus = memory_read(0x50,0x00);
            writeAdress= memStatus + 1;
            writeData = sensorValue;
            memory_write(0x50, 0x00, writeAdress);
            memory_write(0x50, writeAdress, sensorValue);
          
            sprintf(out_buffer, "LDR = %d GRAVADO COM SUCESSO \n", writeData);
            flag_write = 1;
            auto_cont = 0;
      }
  }


   
  if (flag_write == 1) {
    Serial.write(out_buffer);
    buffer_clean();
    flag_write = 0;
  }


  
}

