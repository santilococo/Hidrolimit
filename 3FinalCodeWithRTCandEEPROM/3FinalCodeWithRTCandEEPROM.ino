//Librerias
#include <Time.h>
#include <TimeAlarms.h>
#include <DS1307RTC.h>


//nuevo
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
int  lastDay;

RTC_DS1307 rtc;    //define the Real Time Clock object


//eeprom
#include <EEPROM.h>
#include <MsTimer2.h>

// Pin to be used as power-down sense
int PWR_DWN_PIN = A1;
// The EEPROM address to be used
int EE_ADDR = 0;
int EE_ADDR2 = 1;

//Definimos variables
int electro_in1         = 12;
int electro_in2         = 11;

byte sensorInterrupt    = 0;  // 0 = digital pin 2
const byte sensorPin    = 2;
const byte pulsadorPin  = 3;    // conectar aqui el pulsador entre pin y GND
float calibrationFactor = 7.11;
volatile unsigned int pulseCount;
float flowRate;
float totalLitres;
unsigned long oldTime;

float volumen;
float volumenMaximo = 1; //CAMBIAR ESTO DEPENDIENDO LOS LITROS MÁXIMOS
bool flag = false;

int state = HIGH;      // the current state of the output pin
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin



//del caudalimetro
void pulseCounter()
{
  pulseCount++;
}



//de memoria eeprom
void PWR_DWN_ISR() {
  
  if (analogRead((PWR_DWN_PIN) < 920)) {
    pinMode((PWR_DWN_PIN), OUTPUT);
    digitalWrite((PWR_DWN_PIN), HIGH);
    return;
  }

  pinMode((PWR_DWN_PIN), INPUT);
  
  if (analogRead((PWR_DWN_PIN) > 1000)) {

    //digitalWrite(13, HIGH);
 
    digitalWrite(electro_in1, HIGH);
    digitalWrite(electro_in2, HIGH);
    
    //EEPROM.put(EE_ADDR, volumen); // del volumen
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
  totalLitres       = 0.0;
  oldTime           = 0;


  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  

  //del eeprom
  EEPROM.get(EE_ADDR, volumen);
  //attachInterrupt(PWR_DWN_PIN, PWR_DWN_ISR, FALLING);
  MsTimer2::set( 10, PWR_DWN_ISR );
  MsTimer2::start();


  //del reloj
  EEPROM.get(EE_ADDR, lastDay);
   
}

void loop() {

  //para hacer que el boton actue como un switch:
  reading = digitalRead(pulsadorPin);

  if (reading == HIGH && previous == LOW) {
    if (state == HIGH)
      state = LOW;
    else
      state = HIGH;
  }

  previous = reading;

  //caudalimetro
  if (volumen < volumenMaximo)
    if ((millis() - oldTime) > 1000)  {  // Only process counters once per second
      detachInterrupt(sensorInterrupt);
      flowRate = (((1000.0) / (millis() - oldTime)) * pulseCount) / calibrationFactor;


      // Add the millilitres passed in this second to the cumulative total
      totalLitres += flowRate / 60;
      volumen = totalLitres;

      // unsigned int frac;
      Serial.print (volumen, 3);
      Serial.println (" L");

      // Reset the pulse counter so we can start incrementing again
      pulseCount = 0;

      // Enable the interrupt again now that we've finished sending output
      attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
      oldTime = millis();


      flag = true; //esto me tengo que fijar
    }


  if (volumen >= volumenMaximo ) {
    flag = false;
  }



  if (flag == true && state == LOW) {
    digitalWrite(electro_in1, LOW);
    digitalWrite(electro_in2, LOW);
  }

  else {
    digitalWrite(electro_in1, HIGH);
    digitalWrite(electro_in2, HIGH);
  }




  //reloj, reinicio de volumen cada dia

  DateTime now;
  if (now.day() != lastDay) // this happens exactly once a day.
    volumen = 0; // reset the accumulated volume

  lastDay = now.day();

}
