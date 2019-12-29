/*
 ASPECTOS PARA VER:
 -usar LastDay en EEPROM ADDRESS 2, con el fin de hacer los reinicios por dia,
 ya pude hacerlo con lo del volumen, esto es tarea facil.
*/

//Librerias a utilizar
#include <Time.h>
#include <TimeAlarms.h>
#include <DS1307RTC.h>
#include <SD.h>                                    
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <EEPROM.h>
#include <MsTimer2.h>

RTC_DS1307 rtc;    //defino el RTC


//Definimos variables
int electro_in1         = 12;
int electro_in2         = 11;

byte sensorInterrupt    = 0;  // 0 = digital pin 2
const byte sensorPin    = 2;
const byte pulsadorPin  = 3;    
float calibrationFactor = 7.11;
volatile unsigned int pulseCount;
float flowRate;
float totalLitres;
unsigned long oldTime;

float volumen;
float volumenMaximo = 0.5; 
bool flag = false;

int state = HIGH;      // estado actual del pin de salida
int reading;           // lectura actual del input pin
int previous = LOW;    // lectura anterior del input pin

int  lastDay;

int eeAddress = 0;
int EE_ADDR2 = 1;


//del caudalimetro
void pulseCounter()
{
  pulseCount++;
}



//de memoria eeprom
void flash() {
 
  if (analogRead(A1) < 920) {
    pinMode(A1, OUTPUT);
    digitalWrite(A1, HIGH);
    return;
  }

  pinMode(A1, INPUT);
  
  if (analogRead(A1) > 1000) {
  
    digitalWrite(electro_in1, HIGH);
    digitalWrite(electro_in2, HIGH);

      EEPROM.put(eeAddress, volumen); // del volumen
    //EEPROM.put(EE_ADDR2, lastDay); // del dia
  }
}



void setup() {

  Serial.begin(9600);

  pinMode(electro_in1, OUTPUT);
  pinMode(electro_in2, OUTPUT);
  digitalWrite(electro_in1, HIGH);
  digitalWrite(electro_in2, HIGH);

  pinMode(sensorPin, INPUT);
  pinMode(pulsadorPin, INPUT);  // pulsador entre pin 3 y GND.

  pulseCount        = 0;
  flowRate          = 0.0;
  oldTime           = 0;


  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  

  //del eeprom
  EEPROM.get(eeAddress, volumen);

  MsTimer2::set(10, flash);   //hermoso timer
  MsTimer2::start();

  Serial.print("Litros iniciales: "); //al final no lo termine utilizando
  Serial.print(volumen);  
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  //del reloj
  //EEPROM.get(EE_ADDR, lastDay);

     
}

void loop() {


  /* 
  Todavia no pude arreglar el tema del BOUNCE SWITCH, ver esto, la unica posiblidad a considerar
  no es usando un timer sino haciendolo de manera fisica.
  */
  
  reading = digitalRead(pulsadorPin);   //para hacer que el boton actue como un "switch"

  if (reading == HIGH && previous == LOW) {
    delay(500);
    if (state == HIGH)
      state = LOW;
    else
      state = HIGH;
  }

  previous = reading;

  //caudalimetro
  if (volumen < volumenMaximo && state == LOW)
    if ((millis() - oldTime) > 1000)  {  // Un contador por segundo, utilizando millis 
      detachInterrupt(sensorInterrupt);
      flowRate = (((1000.0) / (millis() - oldTime)) * pulseCount) / calibrationFactor;


      // Llevarlo al total
      volumen += flowRate / 60;
      
      // unsigned int frac;
      Serial.print (volumen, 3);
      Serial.println (" L");

      // Reseteamos para volver a empezar
      pulseCount = 0;

      // Habilitamos la interrupción
      attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
      oldTime = millis();

      flag = true; //esto me tengo que fijar, porque ahora anda pero no se si es lo mas conveniente.
    }


  if (volumen >= volumenMaximo ) {
    flag = false;
  }

  
  //encendido o apagado de electrovalvula
  if (flag == true && state == LOW) {
    digitalWrite(electro_in1, LOW);
    digitalWrite(electro_in2, LOW);
  }

  else {
    digitalWrite(electro_in1, HIGH);
    digitalWrite(electro_in2, HIGH);
  }




  //reloj, reinicio de volumen cada dia
/*
  DateTime now;
  if (now.day() != lastDay) // this happens exactly once a day.
    volumen = 0; // reset the accumulated volume

  lastDay = now.day();
*/
}
