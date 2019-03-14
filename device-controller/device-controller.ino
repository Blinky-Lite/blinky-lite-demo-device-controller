#include "device-controller.h"

const int led13Pin = 13;
boolean led13 = false;
unsigned long nowTime;
boolean heartbeat = false;
int heartbeatPeriod = 1000;
unsigned long startTimeHeartBeat;

const int ledPin = 3;
int ledState = 1;
int ledValue = 255;
int ledValueToggle = 0;
int ledPeriod = 100;
unsigned long startTimeLed = 0;

const int photoDetPin = A0;
const int photoLedPin = 6;
int photoDetValue = 0;
boolean photoLedValue = false;
int photoDetPeriod = 1000;
float photoLedPeriod = 250;
float photoLedPeriodMin = 50;
float photoLedPeriodMax = 2000;
float photoLedMaxBright = 0.70;
unsigned long startTimePhotoDet = 0;
unsigned long startTimePhotoLed = 0;

const int ultraTrigPin = 22;
const int ultraEchoPin = 23;
const int ultraLedPin = 7;
float ultraLedValue = 0;
long ultraDuration = 0;
int ultraDistance = 0;
int ultraAlarmDistance = 10;
int ultraPeriod = 1000;
unsigned long startTimeUltra = 0;

const int interruptTestInPin = 10;
const int interruptTestOutPin = 11;
volatile boolean interruptFlag = false;
volatile int interruptCounter = 0;
const int maxInterruptCounts = 500;

const int tempPin  = A2;
int tempPinValue = 0;
float tempValue = 0.0;
float tempAvg = 0.0;
int tempPeriod = 2000;
unsigned long startTimeTemp = 0;

void setup()
{
  setupCommunications(true, 115200);
  //Serial.begin(9600);
      
  pinMode(led13Pin, OUTPUT); 
  pinMode(ledPin, OUTPUT); 
  pinMode(photoDetPin, INPUT); 
  pinMode(photoLedPin, OUTPUT);
  pinMode(ultraLedPin, OUTPUT);
  pinMode(ultraTrigPin, OUTPUT);
  pinMode(ultraEchoPin, INPUT);
  pinMode(ultraTrigPin, OUTPUT);
  pinMode(interruptTestOutPin, OUTPUT); 
  pinMode(interruptTestInPin, INPUT); 
  pinMode(tempPin, INPUT);
   
  nowTime = millis();
  startTimeHeartBeat = nowTime;
  startTimeLed = nowTime;
  startTimePhotoDet = nowTime;
  startTimeUltra = nowTime;
  startTimeTemp = nowTime;
  
  if (ledState == 2) analogWrite(ledPin, ledValue);    
  attachInterrupt(interruptTestInPin, interruptTestHandler, HIGH);

 }

void loop()
{
  if (dataOnSerial())
  {
    if (getInputTopic().equals("getLedState")) printMessage("ledState", intToString(ledState));
    if (getInputTopic().equals("getLedValue")) printMessage("ledValue", intToString(ledValue));
    if (getInputTopic().equals("getLedPeriod")) printMessage("ledPeriod", intToString(ledPeriod));
    if (getInputTopic().equals("setLedState"))
    {
      ledState = stringToInt(getInputPayload());
      if (ledState == 0) analogWrite(ledPin, 0);
      if (ledState == 2) analogWrite(ledPin, ledValue);
      printMessage("ledState", intToString(ledState));

    }
    if (getInputTopic().equals("setLedValue"))
    {
      ledValue = stringToInt(getInputPayload());
      if (ledState == 2) analogWrite(ledPin, ledValue);
      printMessage("ledValue", intToString(ledValue));
    }
    if (getInputTopic().equals("setLedPeriod")) ledPeriod = stringToInt(getInputPayload());
    if (getInputTopic().equals("setUltraAlarmDistance")) ultraAlarmDistance = stringToInt(getInputPayload());
    
  }
  nowTime = millis();
  if (ledState == 1)
  {
    if ((int)(nowTime - startTimeLed) > ledPeriod)
    {
      startTimeLed = nowTime;
      if (ledValueToggle == 0)
      {
        ledValueToggle = ledValue;
      }
      else
      {
        ledValueToggle = 0;
      }
      analogWrite(ledPin, ledValueToggle);
    }
  }
  
  if ((int)(nowTime - startTimePhotoDet) > photoDetPeriod)
  {
    startTimePhotoDet = nowTime;
    photoDetValue = analogRead(photoDetPin);

    photoLedPeriod = (photoLedPeriodMax - photoLedPeriodMin)  / (photoLedMaxBright * 1024);
    photoLedPeriod = photoLedPeriod * (photoDetValue - (1- photoLedMaxBright) * 1024) + photoLedPeriodMin;
    if (photoLedPeriod < photoLedPeriodMin) photoLedPeriod = photoLedPeriodMin;
   
    printMessage("photoDetValue", intToString(photoDetValue));
  }
  if ((int)(nowTime - startTimePhotoLed) > (int) photoLedPeriod)
  {
    startTimePhotoLed = nowTime;
    photoLedValue = !photoLedValue;
    digitalWrite(photoLedPin, photoLedValue);
  }

  if ((int)(nowTime - startTimeUltra) > ultraPeriod)
  {
    startTimeUltra = nowTime;
    digitalWrite(ultraTrigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(ultraTrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(ultraTrigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    ultraDuration = pulseIn(ultraEchoPin, HIGH);
    // Calculating the distance
    ultraDistance= ultraDuration * 0.034/2;
    ultraLedValue = LOW;
    if (ultraDistance <= ultraAlarmDistance) ultraLedValue = HIGH;
    digitalWrite(ultraLedPin, ultraLedValue);
    printMessage("ultraDistance", intToString(ultraDistance));
  }

  tempPinValue = analogRead(tempPin);
  tempValue = 3.2 * (((float) tempPinValue) / 1024.0);
  tempValue = (215.0 * tempValue / (3.2 - tempValue)) - 100.6;
  tempValue = 2.70 * tempValue;
  tempAvg = tempAvg + (tempValue - tempAvg) / 1000;


  if ((int)(nowTime - startTimeTemp) > tempPeriod)
  {
    startTimeTemp = nowTime;
    printMessage("tempAvg", floatToString(tempAvg,2));
  }
 

  if ((int)(nowTime - startTimeHeartBeat) > heartbeatPeriod)
  {
    startTimeHeartBeat = nowTime;
    heartbeat = !heartbeat;
    led13 = !led13;
    digitalWrite(led13Pin, led13);
    printMessage("heartbeat", booleanToString(heartbeat));
  }
  if(interruptFlag)
  {
    interruptFlag = false;
    interruptCounter = 0;
    digitalWrite(interruptTestOutPin, LOW);
    ledState = ledState + 1;
    if (ledState > 2) ledState = 0;
    if (ledState == 0) analogWrite(ledPin, 0);
    if (ledState == 2) analogWrite(ledPin, ledValue);
    printMessage("ledState", intToString(ledState));
  }

}
void interruptTestHandler() 
{
  if (interruptFlag) return;
  if (interruptCounter < maxInterruptCounts)
  {
    ++interruptCounter;
    return;
  }
  digitalWrite(interruptTestOutPin, HIGH);
  interruptFlag = true;
}
