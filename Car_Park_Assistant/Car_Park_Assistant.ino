#include <HCSR04.h>
#include <Nextion.h>

#define RA_WINDOW_SIZE  30

#define RED_LED_PIN    5
#define GREEN_LED_PIN  6

#define LED_BRIGHTNESS 255

#define EARLY_THRESHOLD   60 // cm
#define OPTIMAL_DISTANCE  40 // cm
#define WARNING_THRESHOLD 20 //cm

#define TRIG_PIN 7
#define ECHO_PIN 8

#define FAST_FLASH_PERIOD 200 // ms
#define SLOW_FLASH_PERIOD 200 // ms

float distRA;
float distHistory[RA_WINDOW_SIZE];

int redLedState = LOW;
int yellowLedState = LOW;
int greenLedState = LOW;

int greenLedToggleTime = 0;
int redLedToggleTime = 0;

#define MOTION_PIN     9
#define MOTION_TIMEOUT 5 // seconds
int motionDetected = LOW;

unsigned long lastMotionDetectedTime = 0;

#define PRINT_INTERVAL 1000 // ms
unsigned long lastPrintTime = 0;

HCSR04 hc(TRIG_PIN,ECHO_PIN);

void allLedsOff() {
  analogWrite(RED_LED_PIN, LOW);
  analogWrite(GREEN_LED_PIN, LOW);
}

void toggleRedLed()
{
  if (redLedState>0) {
    redLedState = LOW;
  } else {
    redLedState = LED_BRIGHTNESS;
  }
  analogWrite(RED_LED_PIN, redLedState);  
}

void toggleGreenLed()
{
  if (greenLedState>0) {
    greenLedState = LOW;
  } else {
    greenLedState = LED_BRIGHTNESS;
  }
  analogWrite(GREEN_LED_PIN, greenLedState); 
}

void slowFlashGreen() {
  analogWrite(RED_LED_PIN, LOW);
  if ((millis() - greenLedToggleTime) > SLOW_FLASH_PERIOD) {
    toggleGreenLed();
    greenLedToggleTime = millis();
  }
}

void solidGreen() {
  analogWrite(RED_LED_PIN, LOW);
  analogWrite(GREEN_LED_PIN, LED_BRIGHTNESS);
}

void solidRed() {
  analogWrite(GREEN_LED_PIN, LOW);
  analogWrite(RED_LED_PIN, LED_BRIGHTNESS);
}

void fastFlashRed() {
  analogWrite(GREEN_LED_PIN, LOW);
  if ((millis() - redLedToggleTime) > FAST_FLASH_PERIOD) {
    toggleRedLed();
    redLedToggleTime = millis();
  }
}


void setup() {
  Serial.begin(9600);
  
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  allLedsOff();
  
  Serial.print("RA_WINDOW_SIZE = ");
  Serial.println(RA_WINDOW_SIZE); 
}

void loop()
{
  checkMotion();
  readDist();
  
//  if ((millis() - lastPrintTime) > PRINT_INTERVAL) {
//    Serial.println("----------------------------");
//    Serial.print("lastPrintTime = ");
//    Serial.print(lastPrintTime);
//    Serial.print(", millis() = ");
//    Serial.print(millis());
//    Serial.print(", delta = ");
//    Serial.println(millis() - lastPrintTime);
//    Serial.print("Distance = ");
//    Serial.println(distRA);
//    Serial.print("MotionDetected = ");
//    Serial.println(motionDetected);
//    lastPrintTime = millis();
//  }

  delay(10);
}

void checkMotion() {
  int motionStatus = digitalRead(MOTION_PIN);

  if (motionStatus) {
    if (!motionDetected) {
      Serial.println("motionDetected = TRUE");
    }
    motionDetected = HIGH;
    lastMotionDetectedTime = millis();
  } else {
    if ((millis() - lastMotionDetectedTime) > (MOTION_TIMEOUT * 1000)) {
      if (motionDetected) {
        Serial.println("motionDetected = FALSE");
      }
      motionDetected = LOW;
    }
  }
  if (motionDetected) {
      digitalWrite(LED_BUILTIN, HIGH);
  } else {
      digitalWrite(LED_BUILTIN, LOW);
  }
}

void calculateRollingAverage(float dist)
{
  static int index = 0;
  static int distHistorySize = 0;

//  Serial.println("====================");
//  Serial.print("distHistorySize = ");
//  Serial.println(distHistorySize);
//  Serial.print("index = ");
//  Serial.println(index);
  
  if (distHistorySize < RA_WINDOW_SIZE)
  {
    distHistorySize++;
  }

  distHistory[index] = dist;

  index++;
  if (index == RA_WINDOW_SIZE)
  {
    index = 0;
  }

  float sum = 0.0;
//  Serial.print("Sum = ");
  for (int i=0;i<distHistorySize;i++)
  {    
//    Serial.print(distHistory[i]);
//    if (i < (distHistorySize - 1)) {
//      Serial.print(" + ");
//    }
    sum += distHistory[i];    
  }
//  Serial.println();

  distRA = sum / distHistorySize;
//  Serial.print("Sum / distHistorySize = ");
//  Serial.print(sum);
//  Serial.print(" / ");
//  Serial.print(distHistorySize);
//  Serial.print(" = ");
//  Serial.println(distRA);
//  Serial.println("====================");
}

void readDist()
{
  float dist = hc.dist();

  calculateRollingAverage(dist);
  
  checkDist();
}

void checkDist()
{
  if (!motionDetected) {
    allLedsOff();
  } else {
    if (distRA > EARLY_THRESHOLD)
    {
      allLedsOff();
    }
    else if ((distRA < EARLY_THRESHOLD) && (distRA > OPTIMAL_DISTANCE))
    {
      solidGreen();
      //slowFlashGreen();
    }
    else if ((distRA < OPTIMAL_DISTANCE) && (distRA > WARNING_THRESHOLD))
    {
      solidRed();
    }
    else if (distRA < WARNING_THRESHOLD)
    {
      fastFlashRed();
    }
  }
  return;
}
