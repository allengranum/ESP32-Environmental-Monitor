
#define RED_LED_1A_PIN    0
#define YELLOW_LED_1A_PIN 1
#define GREEN_LED_1A_PIN  2

#define RED_LED_1B_PIN    3
#define YELLOW_LED_1B_PIN 4
#define GREEN_LED_1B_PIN  5

#define RED_LED_2A_PIN    6
#define YELLOW_LED_2A_PIN 7
#define GREEN_LED_2A_PIN  8

#define RED_LED_2B_PIN    9
#define YELLOW_LED_2B_PIN 10
#define GREEN_LED_2B_PIN  11

#define DELAY 100 // ms

void initLeds() {
  pinMode(RED_LED_1A_PIN, OUTPUT);
  pinMode(YELLOW_LED_1A_PIN, OUTPUT);
  pinMode(GREEN_LED_1A_PIN, OUTPUT);

  pinMode(RED_LED_1B_PIN, OUTPUT);
  pinMode(YELLOW_LED_1B_PIN, OUTPUT);
  pinMode(GREEN_LED_1B_PIN, OUTPUT);

  pinMode(RED_LED_2A_PIN, OUTPUT);
  pinMode(YELLOW_LED_2A_PIN, OUTPUT);
  pinMode(GREEN_LED_2A_PIN, OUTPUT);

  pinMode(RED_LED_2B_PIN, OUTPUT);
  pinMode(YELLOW_LED_2B_PIN, OUTPUT);
  pinMode(GREEN_LED_2B_PIN, OUTPUT);
}

void allLeds(int state) {
  digitalWrite(RED_LED_1A_PIN, state);  
  digitalWrite(YELLOW_LED_1A_PIN, state);  
  digitalWrite(GREEN_LED_1A_PIN, state);  

  digitalWrite(RED_LED_1B_PIN, state);  
  digitalWrite(YELLOW_LED_1B_PIN, state);  
  digitalWrite(GREEN_LED_1B_PIN, state);
  
  digitalWrite(RED_LED_2A_PIN, state);  
  digitalWrite(YELLOW_LED_2A_PIN, state);  
  digitalWrite(GREEN_LED_2A_PIN, state);

  digitalWrite(RED_LED_2B_PIN, state);  
  digitalWrite(YELLOW_LED_2B_PIN, state);  
  digitalWrite(GREEN_LED_2B_PIN, state);
}

void allRedLeds(int state) {
  digitalWrite(RED_LED_1A_PIN, state);  
  digitalWrite(RED_LED_1B_PIN, state);  
  digitalWrite(RED_LED_2A_PIN, state);  
  digitalWrite(RED_LED_2B_PIN, state);  
}

void allYellowLeds(int state) {
  digitalWrite(YELLOW_LED_1A_PIN, state);  
  digitalWrite(YELLOW_LED_1B_PIN, state);  
  digitalWrite(YELLOW_LED_2A_PIN, state);  
  digitalWrite(YELLOW_LED_2B_PIN, state);  
}

void allGreenLeds(int state) {
  digitalWrite(GREEN_LED_1A_PIN, state);  
  digitalWrite(GREEN_LED_1B_PIN, state);
  digitalWrite(GREEN_LED_2A_PIN, state);
  digitalWrite(GREEN_LED_2B_PIN, state);
}


void setup() {
  initLeds();
  allLeds(LOW);
}


void loop() {
  allRedLeds(HIGH);
  delay(DELAY);
  
  allRedLeds(LOW);
  allYellowLeds(HIGH);
  delay(DELAY);

  allYellowLeds(LOW);
  allGreenLeds(HIGH);
  delay(DELAY);
  
  allGreenLeds(LOW);
}
