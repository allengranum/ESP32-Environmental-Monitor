
#define RA_WINDOW_SIZE_SEC             30
#define TEMPERATURE_POLLING_PERIOD_SEC  1
#define TIME_BUFFER                    30 /* seconds */

#define LED_PIN 16

int threshold = 24.5;

int LM35_PIN= A0;

float tempRA;
float tempHistory[RA_WINDOW_SIZE_SEC/TEMPERATURE_POLLING_PERIOD_SEC];
int numSamples = RA_WINDOW_SIZE_SEC/TEMPERATURE_POLLING_PERIOD_SEC;

enum FanState {OFF, ON, SHUTTING_DOWN};

int ledState = LOW;
enum FanState state = OFF;
int startTime = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(LED_PIN, OUTPUT);
  
  numSamples = RA_WINDOW_SIZE_SEC/TEMPERATURE_POLLING_PERIOD_SEC;
  Serial.print("Rolling average window size (seconds) = ");
  Serial.println(RA_WINDOW_SIZE_SEC);
  Serial.print("Temperature polling period = ");
  Serial.println(TEMPERATURE_POLLING_PERIOD_SEC);
  Serial.print("numSamples = ");
  Serial.println(numSamples);
  digitalWrite(LED_PIN, LOW);
}

void loop()
{
  Serial.println("--------------------------------");
  Serial.print("Led state = ");
  Serial.println(ledState);
  readTemp();
  delay(TEMPERATURE_POLLING_PERIOD_SEC * 1000);
}

void toggleLed()
{
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);  
}

void calculateRollingAverage(float temp)
{
  static int index = 0;
  static int tempHistorySize = 0;

  Serial.print("tempHistorySize = ");
  Serial.println(tempHistorySize);
  Serial.print("index = ");
  Serial.println(index);
  
  if (tempHistorySize < (RA_WINDOW_SIZE_SEC/TEMPERATURE_POLLING_PERIOD_SEC))
  {
    tempHistorySize++;
  }

  tempHistory[index] = temp;

  index++;
  if (index == numSamples)
  {
    Serial.println("Resetting index to 0");
    index = 0;
  }

  float sum = 0.0;
  for (int i=0;i<tempHistorySize;i++)
  {
    sum += tempHistory[i];
  }
  tempRA = sum / tempHistorySize;
  Serial.print("tempRA = ");
  Serial.println(tempRA);
}

void readTemp()
{
  int analogValue = analogRead(LM35_PIN);
  float millivolts = analogValue * (1024.0 / 1000);
  float celcius = millivolts/10;

  Serial.print("temp = ");
  Serial.println(celcius);
  calculateRollingAverage(celcius);
  checkTemp();
}

void checkTemp()
{
  if ((tempRA > threshold) && (state == OFF))
  {
    Serial.println("Turning fan on");
    state = ON;
    digitalWrite(LED_PIN, HIGH);
  }
  else if ((tempRA < threshold) && (state == ON))
  {
    Serial.println("Shutting down in 30 seconds");
    state = SHUTTING_DOWN;
    startTime = millis();
  }
  else if ((tempRA < threshold) && (state == SHUTTING_DOWN))
  {
    if ((millis() - startTime) > (TIME_BUFFER * 1000))
    {
      Serial.println("Turning fan off");
      state = OFF;
      digitalWrite(LED_PIN, LOW);
    } else {
      Serial.print("shutdown timer: ");
      Serial.println((millis() - startTime)/1000);
    }
  }
  else if ((tempRA > threshold) && (state == SHUTTING_DOWN))
  {
    Serial.println("Temp went back up. Cancelling shutdown timer.");
    state = ON;
  }
  Serial.print("fan state = ");
  Serial.println(state);
  return;
}
