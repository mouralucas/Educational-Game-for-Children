////////////////include////////////////////
// include para som do buzzer
#include "Mario.h"
#include "Underworld.h"
#include "Musica.h"
#include <Servo.h>
#include <SoftwareSerial.h>  

///////////////Bluetooth//////////////////
// pinos de dados do bluetooth
#define rxPin 12
#define txPin 13

boolean receivedControl = false;  // boolean de controle para quando a mensagem estiver completa

// Biblioteca que define a variavel serial para o uso do bluetooth
SoftwareSerial bluetoothSerial(rxPin, txPin); // RX, TX   

////////////////Servos/////////////////////
int pinEyeLeft = 11;
int pinEyeRight = 10;
int pinMouth =9;
Servo eyebrow_left;
Servo eyebrow_right;
Servo mouth;

////////////////sensores IR/////////////////
#define sensorIR_intersec A0
#define sensorIR_left A1
#define sensorIR_central A2
#define sensorIR_right A3
#define sensorIR_turn A4

///////////////controle pid //////////////////
const float kp = 100; 
const float kd = 4;
const float ki = 1; 

int correction;
int error;
int lastError;
int integral;
int derivative;
int speedLeft;
int speedRight;


////////////////motores DC///////////////////
int motorDireito = 3;  
int entradaD1 = 2;
int entradaD2 = 4; 

int motorEsquerdo = 5;  
int entradaE1 = 6;
int entradaE2 = 7; 


//int speedMotorRight = 100;
//int speedMotorLeft = 100;
const int speedMotorSetUp = 255;//70;
const int speedMotorCurva = 255;

/////////////////Fim de curso ////////////////
int fimDeCurso = 8;

/////////////////Buzzer/////////////////////
#define Buzzer A5
//int Buzzer = 1;
// variaveis para execucao da musica, Mario == feliz, Underworld == triste
Mario mSong;
Underworld uSong;

////////////////Strings/////////////////////
const char endChar = 'e';
const String charLeft = "l";
const String charRight = "r";
const String charAhead = "a";

String inputString;         // a string que conterah a mensagem recebida
int index;




//////////////////outros

//boolean turnOn;
 boolean incompletePath;
//////

void setup() 
{ 
  Serial.begin(9600);

  //////////////////bluetooth/////////////////////
  setupBluetooth();
    //////////////////motores DC//////////////////
  pinMode(motorDireito, OUTPUT);  
  pinMode(entradaD1, OUTPUT);  
  pinMode(entradaD2, OUTPUT);  
  
  pinMode(motorEsquerdo, OUTPUT);  
  pinMode(entradaE1, OUTPUT);  
  pinMode(entradaE2, OUTPUT);  

  //////////////// sensor IR //////////////////////
  pinMode(sensorIR_intersec, INPUT_PULLUP);
  pinMode(sensorIR_left, INPUT_PULLUP);
  pinMode(sensorIR_central, INPUT_PULLUP);
  pinMode(sensorIR_right, INPUT_PULLUP);
  pinMode(sensorIR_turn, INPUT_PULLUP);
  
  ///////////////servos///////////////////////////////
  eyebrow_left.attach(pinEyeLeft);
  eyebrow_right.attach(pinEyeRight);
  mouth.attach(pinMouth);

//robo começa feliz
  eyebrow_right.write(15);
  eyebrow_left.write(110); 
  mouth.write(130);
    
  
  /////////////////fim de curso ////////////////////
  pinMode(fimDeCurso, INPUT);

  /////////////////buzzer/////////////////////////
  pinMode(Buzzer, OUTPUT); //buzzer

  mSong.pinDefine(Buzzer);
  uSong.pinDefine(Buzzer);
  
  ////////////// OUTROS ///////////////////////
  initializeControl();
  stopMotor();
  index = 0;
  incompletePath = true;

  inputString = "";
  //turnOn = false;
} 
void loop() 
{ 
    
    readSerialBluetooth();
    
    if(receivedControl){
         index = 0;
        //robo começa feliz
         eyebrow_right.write(15);
         eyebrow_left.write(110); 
         mouth.write(130);
         incompletePath = true;
         for (index; index < inputString.length(); index++) {
            String comand = inputString.substring(index, (index + 1));
            Serial.print("a substring eh: ");
            Serial.println(comand);
            
            if (comand == charAhead) {
              Serial.println("vai pra frente");
              moveAhead();
              /*for(int i =0; i<50;i++){
                pidController();
              }*/
              delay(400);
              while( detectIntersection() == false){
                //Serial.println("Estou no while do moveAhead");
                pidController();
                if(digitalRead(fimDeCurso) == HIGH)
                  barrier();
                
              }
              
              //anda um pouquinho
              analogWrite(motorEsquerdo, speedMotorSetUp);
              analogWrite(motorDireito, speedMotorSetUp);
              moveAhead();
              delay(180);
              stopMotor();
              
            /*
            // se o prox. comando nao for ir pra frente, volta um pouco
             if(((index+1) < inputString.length()) && inputString.substring((index+1), (index + 2))!="a"){
                Serial.println("volta um pouquinho");
                moveBack();
                //while( detectIntersection() == false);
                delay(200);
                stopMotor();
            }  
            */  
            }
            else if (comand == charRight) {
              Serial.println("vai pra direita");
              moveRight();
              delay(700);
              while(detectStreet() == false);
              
              
            }
            else if( comand == charLeft) {
              Serial.println("vai pra esquerda");
              moveLeft(); 
              delay(700);
              while(detectStreet() == false);
                       
            }
            
            delay(1000);
            initializeControl();
        }
        if(digitalRead(sensorIR_intersec)==HIGH && detectIntersection()) 
          complete();
        if(incompletePath){
          incomplete();
        }
      }
  
  
} 

////////////////////FUNCOES PID//////////////////////////////////
void initializeControl(){
   error = 0;
   lastError = 0;
   integral = 0;
   derivative = 0;
   speedLeft = speedMotorSetUp;
   speedRight = speedMotorSetUp;
   analogWrite(motorEsquerdo, speedLeft);
   analogWrite(motorDireito, speedRight);
}

void pidController(){
    boolean readSensorRight = digitalRead(sensorIR_right);
    boolean readSensorLeft = digitalRead(sensorIR_left);

    //O Robô está indo para a esquerda, por isso o sensor da direita está sobre a linha   
    if(readSensorRight && !readSensorLeft){
      error = 1;
      Serial.println(" Indo pra esquerda ");
    }
    //O Robô está indo para a direita, por isso o sensor da esquerda está sobre a linha   
    else if(readSensorLeft && !readSensorRight){
      error = -1;
      Serial.println(" Indo pra direita ");
      
    }
    //O Robô está sobre a linha 
    else{
      error = 0;
      Serial.println(" Indo certo! ");
      speedLeft = speedMotorSetUp;
      speedRight = speedMotorSetUp;
      
    }
    

    derivative = error - lastError;
    integral += error;
    correction = (int)(error*kp  + derivative*kd + integral*ki);
    
    lastError = error;
   

     
     
     
     if(error !=0){
       Serial.println("corrigindo.......... ");
          //O Robô está indo para a esquerda, diminui a velocidade do motor direito para corrigir
          if(correction > 0) 
          {
          
            speedLeft = speedMotorSetUp;
            speedRight = correction;      
          }
          /*O Robô está indo para a direita, diminui a velocidade do motor esquerdo para corrigir
          Por isso a velocidade é speedMotorSetUp + ( correction), já que correction tem valor negativo */
          else if (correction < 0){ 
           
            speedLeft = (-1* correction);
            speedRight = speedMotorSetUp;
          }
          //Se correction == 0, não tem pq mexer nos motores 
     }
     
    
    Serial.print("Motor esquerdo: ");
    Serial.println(speedLeft);
    Serial.print("Motor direito: ");
    Serial.println(speedRight);
    
    Serial.println("\n\n\n"); 
    
    analogWrite(motorEsquerdo, speedLeft);
    analogWrite(motorDireito, speedRight);
}

///////////////////INTERSECAO////////////////////////////////
boolean detectIntersection(){
  if(digitalRead(sensorIR_left)== HIGH &&
     digitalRead(sensorIR_right)== HIGH &&
     digitalRead(sensorIR_central)== HIGH ){
  
        stopMotor();
        return true;
     }
     return false;
}


boolean detectStreet(){
  if(digitalRead(sensorIR_turn) == HIGH){
       stopMotor();
      return true;
  }
  return false;
}

/////////////// FUNCOES FEEDBACK//////////////////////////
//metodos de controle de curso
 void complete(){
  //metodo para acionar os feedbacks de fim de curso
  //turnOn = false;
  incompletePath = false;
  index = inputString.length();
  
  // envia mensagem de percurso completo via bluetooth 
  sendMessageWithBluetooth(1);

  eyebrow_right.write(15);
  eyebrow_left.write(110); 
  mouth.write(130);
    
  // musica feliz, deixar para executar por ultimo (possivelmete eh sincrono)
  mSong.sing();
   
}

void incomplete(){
  //metodo para acionar os feedbacks de falha no caminho
    //turnOn=false;
    index = inputString.length();

    
  // envia mensagem de percurso incompleto via bluetooth 
  sendMessageWithBluetooth(2);
  
  eyebrow_right.write(90);
  eyebrow_left.write(70);
  mouth.write(160);
  
  // musica triste, deixar para executar por ultimo (possivelmete eh sincrono)
  uSong.sing();
  
  incompletePath = true;
}

void barrier(){
  //metodo para acionar os feedbacks de obstaculo
  stopMotor();
  //turnOn=false;
  index = inputString.length();
  incompletePath = false;
  
  // envia mensagem de colisao via bluetooth 
  sendMessageWithBluetooth(3);

  eyebrow_right.write(90);
  eyebrow_left.write(70);
  mouth.write(160);
  
  // musica triste, deixar para executar por ultimo (possivelmete eh sincrono)
   uSong.sing();
}



/////////////////FUNCOES MOTORES ////////////////////////////
void stopMotor(){

  //Desliga o Motor 1
   digitalWrite(entradaD1, LOW);  
   digitalWrite(entradaD2, LOW);
    
   //Desliga o Motor 2
   digitalWrite(entradaE1, LOW);  
   digitalWrite(entradaE2, LOW); 
}

void moveAhead(){

  //incluir uma chamada de função pra correção de curso
  //Aciona o Motor 1
   digitalWrite(entradaD1, LOW);  
   digitalWrite(entradaD2, HIGH);
    
   //Aciona o Motor 2
   digitalWrite(entradaE1, LOW);  
   digitalWrite(entradaE2, HIGH); 
}

void moveBack(){

  //incluir uma chamada de função pra correção de curso
  //Aciona o Motor 1
   digitalWrite(entradaD1, HIGH);  
   digitalWrite(entradaD2, LOW);
    
   //Aciona o Motor 2
   digitalWrite(entradaE1, HIGH);  
   digitalWrite(entradaE2, LOW); 
}

void moveRight(){

  analogWrite(motorEsquerdo, speedMotorCurva);
  analogWrite(motorDireito, speedMotorCurva);

   //Aciona o Motor 1
   digitalWrite(entradaD1, HIGH);  
   digitalWrite(entradaD2, LOW);
    
   //Aciona o Motor 2
   digitalWrite(entradaE1, LOW);  
   digitalWrite(entradaE2, HIGH); 
}

void moveLeft(){
  analogWrite(motorEsquerdo, speedMotorCurva);
  analogWrite(motorDireito, speedMotorCurva);
  
   //Aciona o Motor 1
   digitalWrite(entradaD1, LOW);  
   digitalWrite(entradaD2, HIGH);
    
   //Aciona o Motor 2
   digitalWrite(entradaE1, HIGH);  
   digitalWrite(entradaE2, LOW); 
}

////////////////////////////////// Bluetooth///////////////////////////////////////
/**
 * O setupBluetooth deve ser chamado no setup do projeto. Essa funcao aloca o tamanho da string, e se preenche as mensagem de saldacao caso a conexao seja estabelicida
 * no log do arduino aparece 'oioioi'
 * no received do android aparece 'Hello from Arduino'
 */
void setupBluetooth() {
  // aloca 200 bytes para a string
  inputString.reserve(200);
  bluetoothSerial.begin(9600);
  bluetoothSerial.println("Hello from Arduino");
  //Serial.println("oioioi");  
}

/**
 * Essa funcao recebe os dados do bluetooth e converte os char recebido em uma string a cada loop do projeto ele intera mais um caracter.
 * Quando o caracter terminal eh encontrado seta o boolean de controle como TRUE, com isso eh garantido que toda a mensagem foi recebida.
 */
void readSerialBluetooth() {
 
  if (bluetoothSerial.available()) {
    Serial.println("Recebi algo");
 
   char inChar = (char) bluetoothSerial.read();
 
    inputString += inChar;

    // procura caracter terminal, nesse caso 'e'
    if (inChar == endChar) {
      receivedControl = true;
    }
    
  Serial.print("recebi: ");
  Serial.println(inputString);
  }
}


void sendMessageWithBluetooth(int option) {

  if (option == 1) {
    bluetoothSerial.println("ARRIVED");
    
  }
  else if (option == 2) {
    bluetoothSerial.println("INCOMPLETE");
    
  }
  else if (option == 3) {
    bluetoothSerial.println("CHASH");
    
  }
  else
    bluetoothSerial.println("Deu treta");

    // reseta variaveis de controle
    receivedControl = false;
    inputString = "";
    
}

