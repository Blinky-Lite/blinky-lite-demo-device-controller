const int led13Pin = 13;
const int ledPin = 3;
const int photoDetPin = A0;
const int photoLedPin = 6;
const int ultraTrigPin = 22;
const int ultraEchoPin = 23;
const int ultraLedPin = 7;
const int tempPin  = A2;
const int interruptTestInPin = 10;
const int interruptTestOutPin = 11;

boolean led13 = false;
unsigned long nowTime;

int ledValueToggle = 0;
unsigned long startTimeLed = 0;

boolean photoLedValue = false;
float photoLedPeriod = 250;
float photoLedPeriodMin = 50;
float photoLedPeriodMax = 2000;
float photoLedMaxBright = 0.70;
unsigned long startTimePhotoLed = 0;

float ultraLedValue = 0;
long ultraDuration = 0;

volatile boolean interruptFlag = false;
volatile int interruptCounter = 0;
const int maxInterruptCounts = 500;

int tempPinValue = 0;
float tempValue = 0.0;

boolean motherShipConnection = false;

struct Readings
{
  int photoDetValue;
  int ultraDistance;
  int ultraAlarmDistance;
  float tempAvg;
  int ledState;
  int ledValue;
  int ledPeriod;
};
Readings readings;

struct Settings
{
  int echoSettings;
  int ledState;
  int ledValue;
  int ledPeriod;
  int ultraAlarmDistance;
};
Settings settings;

struct SettingsMask
{
  int echoSettings;
  int ledState;
  int ledValue;
  int ledPeriod;
  int ultraAlarmDistance;
};
SettingsMask settingsMask;

void setup()
{
  Serial1.begin(115200);
  Serial.begin(9600);
      
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
  startTimeLed = nowTime;
  startTimePhotoLed = nowTime;
  readings.tempAvg = 0.0;
  readings.ledState = 1;
  readings.ledValue = 255;
  readings.ledPeriod = 100;
  readings.ultraAlarmDistance = 10;

  settings.echoSettings = 0;
  digitalWrite(led13Pin, led13);
  digitalWrite(photoLedPin, photoLedValue);
  
  if (readings.ledState == 2) analogWrite(ledPin, readings.ledValue);    
  attachInterrupt(interruptTestInPin, interruptTestHandler, HIGH);
}

void loop()
{
  while(Serial1.available() > 0)
  {
    Serial1.readBytes((uint8_t*) &settingsMask,sizeof(settingsMask));
    Serial1.readBytes((uint8_t*) &settings, sizeof(settings));
    motherShipConnection = true;
  }
  if (settingsMask.echoSettings > 0)
  {
     settingsMask.echoSettings = 0;
     updateReadings();
     Serial1.write((uint8_t*)&readings, sizeof(readings));
  }
  if (settingsMask.ledState > 0)
  {
    if((0 <= settings.ledState) && (settings.ledState <= 2))
    {
      readings.ledState = settings.ledState;
      if (readings.ledState == 0) analogWrite(ledPin, 0);
      if (readings.ledState == 2) analogWrite(ledPin, readings.ledValue);
    }
    settingsMask.ledState = 0;
  }
  if (settingsMask.ledValue > 0)
  {
    if((0 <= settings.ledValue) && (settings.ledValue <= 255))
    {
      readings.ledValue = settings.ledValue;
      if (readings.ledState == 2) analogWrite(ledPin, readings.ledValue);
    }
    settingsMask.ledValue = 0;
  }
  if (settingsMask.ledPeriod > 0)
  {
    if((100 <= settings.ledPeriod) && (settings.ledPeriod <= 10000))
    {
      readings.ledPeriod = settings.ledPeriod;
    }
    settingsMask.ledPeriod = 0;
  }
  if (settingsMask.ultraAlarmDistance > 0)
  {
    if((1 <= settings.ultraAlarmDistance) && (settings.ultraAlarmDistance <= 1000))
    {
      readings.ultraAlarmDistance = settings.ultraAlarmDistance;
    }
    settingsMask.ultraAlarmDistance = 0;
  }

  if (motherShipConnection) nowTime = millis();
  if (readings.ledState == 1)
  {
    if ((int)(nowTime - startTimeLed) > readings.ledPeriod)
    {
      startTimeLed = nowTime;
      if (ledValueToggle == 0)
      {
        ledValueToggle = readings.ledValue;
      }
      else
      {
        ledValueToggle = 0;
      }
      analogWrite(ledPin, ledValueToggle);
    }
  }
  
  if ((int)(nowTime - startTimePhotoLed) > (int) photoLedPeriod)
  {
    startTimePhotoLed = nowTime;
    photoLedValue = !photoLedValue;
    digitalWrite(photoLedPin, photoLedValue);
  }

  tempPinValue = analogRead(tempPin);
  tempValue = 3.2 * (((float) tempPinValue) / 1024.0);
  tempValue = (215.0 * tempValue / (3.2 - tempValue)) - 100.6;
  tempValue = 2.70 * tempValue;
  readings.tempAvg = readings.tempAvg + (tempValue - readings.tempAvg) / 1000;
 
  if(interruptFlag)
  {
    interruptFlag = false;
    interruptCounter = 0;
    digitalWrite(interruptTestOutPin, LOW);
    readings.ledState = readings.ledState + 1;
    if (readings.ledState > 2) readings.ledState = 0;
    if (readings.ledState == 0) analogWrite(ledPin, 0);
    if (readings.ledState == 2) analogWrite(ledPin, readings.ledValue);
//    printMessage("ledState", intToString(readings.ledState));
  }

}
void updateReadings()
{
    led13 = !led13;
    digitalWrite(led13Pin, led13);

    readings.photoDetValue = analogRead(photoDetPin);
    photoLedPeriod = (photoLedPeriodMax - photoLedPeriodMin)  / (photoLedMaxBright * 1024);
    photoLedPeriod = photoLedPeriod * (readings.photoDetValue - (1- photoLedMaxBright) * 1024) + photoLedPeriodMin;
    if (photoLedPeriod < photoLedPeriodMin) photoLedPeriod = photoLedPeriodMin;
    readings.photoDetValue = (1024 - readings.photoDetValue) / 10;

    digitalWrite(ultraTrigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(ultraTrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(ultraTrigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    ultraDuration = pulseIn(ultraEchoPin, HIGH);
    // Calculating the distance
    readings.ultraDistance= ultraDuration * 0.034/2;
    ultraLedValue = LOW;
    if (readings.ultraDistance <= readings.ultraAlarmDistance) ultraLedValue = HIGH;
    digitalWrite(ultraLedPin, ultraLedValue);  
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
