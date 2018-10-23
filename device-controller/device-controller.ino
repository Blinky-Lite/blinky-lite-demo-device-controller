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

const int buttonPin = 2;
int buttonBuffer[2] = {LOW, LOW};
int buttonBufferPointer[2] = {1, 0};
int buttonPeriod = 10;
unsigned long startTimeButton = 0;

const int photoDetPin = A0;
int photoDetValue = 0;
int photoDetPeriod = 1000;
unsigned long startTimePhotoDet = 0;

const int interruptTestOutPin = 9;
const int interruptTestInPin = 10;
const int interruptTestPin = 11;
boolean interruptTestOutValue = false;
volatile boolean interruptFlag = false;
void setup()
{
  setupCommunications(true, 115200);
  //Serial.begin(9600);
      
  pinMode(led13Pin, OUTPUT); 
  pinMode(ledPin, OUTPUT); 
  pinMode(buttonPin, INPUT); 
  pinMode(photoDetPin, INPUT); 
  pinMode(interruptTestOutPin, OUTPUT); 
  pinMode(interruptTestInPin, INPUT_PULLUP); 
  pinMode(interruptTestPin, OUTPUT); 
   
  nowTime = millis();
  startTimeHeartBeat = nowTime;
  startTimeLed = nowTime;
  startTimeButton = nowTime;
  startTimePhotoDet = nowTime;
  
  if (ledState == 2) analogWrite(ledPin, ledValue);    
  attachInterrupt(interruptTestInPin, interruptTestRising, RISING);
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
    if (getInputTopic().equals("setLedPeriod"))ledPeriod = stringToInt(getInputPayload());
    
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

  if ((int)(nowTime - startTimeButton) > buttonPeriod)
  {
    startTimeButton = nowTime;
    buttonBuffer[buttonBufferPointer[0]] = digitalRead(buttonPin);
    if ((buttonBuffer[buttonBufferPointer[0]] == HIGH)  && (buttonBuffer[buttonBufferPointer[1]] == LOW) )
    {
      ledState = ledState + 1;
      if (ledState > 2) ledState = 0;
      if (ledState == 0) analogWrite(ledPin, 0);
      if (ledState == 2) analogWrite(ledPin, ledValue);
      printMessage("ledState", intToString(ledState));
    }
    for (int ii = 0; ii < 2; ++ii)
    {
        ++buttonBufferPointer[ii]; 
        if (buttonBufferPointer[ii] > 1) buttonBufferPointer[ii]  = 0;
    }
  }
  
  if ((int)(nowTime - startTimePhotoDet) > photoDetPeriod)
  {
    startTimePhotoDet = nowTime;
    photoDetValue = analogRead(photoDetPin);
    printMessage("photoDetValue", intToString(photoDetValue));
  }


  if ((int)(nowTime - startTimeHeartBeat) > heartbeatPeriod)
  {
    startTimeHeartBeat = nowTime;
    heartbeat = !heartbeat;
    led13 = !led13;
    digitalWrite(led13Pin, led13);
    printMessage("heartbeat", booleanToString(heartbeat));
  }
    interruptTestOutValue = !interruptTestOutValue;
    digitalWrite(interruptTestOutPin, interruptTestOutValue);
    if (!interruptTestOutValue) digitalWrite(interruptTestPin, LOW);

}
void interruptTestRising() 
{
  digitalWrite(interruptTestPin, HIGH);
}


