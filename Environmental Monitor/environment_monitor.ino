#include "environment_monitor.h"
#include "DeviceInfo.h"
#include "esp_onboarding.h"
#include "esp_config.h"
#include "esp_mqtt.h"
#include "UiMgr.h"
#include "EnvMonConfig.h"
#include "GpioMgr.h"
#include "ConnMgr.h"
#include "EspPins.h"
//#include "bitmaps.h"

#include <SPI.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

#include <FreeSans12pt7b.h>
#include <FreeSans12pt7b.h>
#include <FreeSans24pt7b.h>
#include <fonts/FreeSansBold24pt7b.h>
#include <fonts/FreeSans9pt7b.h>

#define CONFIG_FILE_NAME    "/thing.conf"    // ToDo I need the / character???

#define FAST_FLASH_uS 500000
#define DATA_READ_INTERVAL_SECONDS 30

// Instantiate the components of the software
DeviceInfo   myDeviceInfo;
UiMgr        uiMgr;
GpioMgr      gpioMgr;
ConnMgr      connMgr;
EnvMonConfig envMonConfig;

//
// NTP & time stuff
//
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

float g_currentTemp;
float g_currentHumidity;

String date, oldDate;
String currentTime, oldTime;
const char * days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;

#define MAX_WIFI_CONNECT_ATTEMPTS 3
#define WIFI_CONNECT_DELAY_MS 500


hw_timer_t * dataCollectionTimer = NULL;
hw_timer_t * wifiConnectTimer = NULL;
hw_timer_t * statusLedTimer = NULL;


bool timeToSendData;
bool timeToConnectToWifi;
int wifiAttemptCount;
char* g_deviceName = NULL;
bool currentMotionDetectedState = false;
bool timeToSendMotionData = true;
long unsigned int noMotionDelay = 10000;
long unsigned int motionDetectedTime;
boolean g_mqttConnected = false;
int mqttReconnectCount = 0;
int wifiReconnectCount = 0;
//int g_orientation = DISPLAY_ORIENTATION_0;
//uint32_t g_statusLedColour;
//int g_statusLedState;
//int g_statusLedBrightness;
//int g_displayBacklightBrightness;
//int g_displayBacklightState;

WiFiClient espClient;


void dataCollectionTimerCallback() {
  timeToSendData = true;
}

void statusLedTimerCallback() {  
  if(LED_ON == myDeviceInfo.getStatusLedState() ) {
    myDeviceInfo.setStatusLedState(LED_OFF);
  } else {
    myDeviceInfo.setStatusLedState(LED_ON);
  }
}

void wifiConnectCallback() {
      timeToConnectToWifi = true;
}

void configButtonPressHandler() {
  Serial.println("CONFIG_BUTTON pressed!!!");
  //deleteConfigData();
}


void _configInit() {
  myDeviceInfo.setStatusLedBrightness( envMonConfig.getStatusLedBrightness() );
  myDeviceInfo.setTemperatureCalibrationValue( envMonConfig.getTemperatureCalibrationValue() );
  myDeviceInfo.setHumidityCalibrationValue( envMonConfig.getHumidityCalibrationValue() );
}

void _statusLedInit() {  
  myDeviceInfo.setStatusLedColour(NEOCOLOUR_RED);
}

void _displayInit() {
  uiMgr.setDisplayOrientation(DISPLAY_ORIENTATION_0);
  uiMgr.setBackgroundColour(COLOR_BLACK);
  uiMgr.showStartingScreen();
  uiMgr.setBacklightBrightness(envMonConfig.getDisplayBackligtBrightness());
  uiMgr.showDeviceName(envMonConfig.getDeviceName());
}

void _gpioInit() {
  pinMode(DHT_SENSOR_PIN, INPUT);
  pinMode(CONFIG_BUTTON, INPUT_PULLUP);
  pinMode(DELETE_CONFIG_BUTTON, INPUT_PULLUP);
  // pinMode(STATUS_LED, OUTPUT);
  pinMode(MOTION_SENSOR_PIN, INPUT);
  delay(100); // There needs to be some delay here before immediately performing
              // a digitalRead on a pin or else the results are non deterministic
}

void _buttonsInit() {
  int configButtonState = digitalRead(CONFIG_BUTTON);
  int deleteConfigButtonState = digitalRead(DELETE_CONFIG_BUTTON);
  if (deleteConfigButtonState == LOW) {
    Serial.printf("\n\nDelete config button pressed during boot.\n");
    Serial.printf("Deleting saved onboarding data...\n");
    envMonConfig.deleteConfigData();
  }
  if (configButtonState == LOW) {
    Serial.printf("\n\nConfig button pressed during boot.\n");
    Serial.printf("    Performing onboarding...\n");
    performOnboarding();
  }
  attachInterrupt(CONFIG_BUTTON, configButtonPressHandler, FALLING);
  Serial.printf("CONFIG_BUTTON ISR attached.\n");
}

void _motionSensorInit() {
  Serial.printf("\n\ncalibrating...\n");
  int motionSensorCalibrationTime = 5;
  digitalWrite(MOTION_SENSOR_PIN, LOW);
  delay(motionSensorCalibrationTime * 1000);
  Serial.printf("Calibration complete.\n");
}

void _flashStatusLed(int onTime, int offTime) {
  // Flash the status LED during setup
  statusLedTimer = timerBegin(2, 80, true);
  timerAttachInterrupt(statusLedTimer, &statusLedTimerCallback, true);
  timerAlarmWrite(statusLedTimer, FAST_FLASH_uS, true);
//  timerAlarmEnable(statusLedTimer);
}


void setup() {

  Serial.begin(9600);

  myDeviceInfo.begin();

  envMonConfig.begin();

  uiMgr.begin();

  gpioMgr.begin();

  connMgr.begin();

  _configInit();

  _statusLedInit();

  _gpioInit();

  _displayInit();

  _buttonsInit();

  _motionSensorInit();

  _flashStatusLed(100, 100);

  delay(1000); // this is magical. if you take it out, things go bad
  envMonConfig.dumpConfig();

  connMgr.connect();

  connectToWifi();
  
  // tft.fillRectangle(TEMP_BLOCK_X, TEMP_BLOCK_Y, TEMP_BLOCK_X + TEMP_BLOCK_WIDTH, TEMP_BLOCK_Y + TEMP_BLOCK_HEIGHT, COLOR_ORANGE);
  // tft.fillRectangle(HUMIDITY_BLOCK_X, HUMIDITY_BLOCK_Y, HUMIDITY_BLOCK_X + HUMIDITY_BLOCK_WIDTH, HUMIDITY_BLOCK_Y + HUMIDITY_BLOCK_HEIGHT, COLOR_SILVER);
  // tft.fillRectangle(TIME_BLOCK_X, TIME_BLOCK_Y, TIME_BLOCK_X + TIME_BLOCK_WIDTH, TIME_BLOCK_Y+TIME_BLOCK_HEIGHT, COLOR_SKYBLUE);
  // tft.fillRectangle(DATE_BLOCK_X, DATE_BLOCK_Y, DATE_BLOCK_X + DATE_BLOCK_WIDTH, DATE_BLOCK_Y+DATE_BLOCK_HEIGHT, COLOR_GREEN);
  // tft.fillRectangle(DEVICE_NAME_BLOCK_X, DEVICE_NAME_BLOCK_Y, DEVICE_NAME_BLOCK_X + DEVICE_NAME_BLOCK_WIDTH, DEVICE_NAME_BLOCK_Y + DEVICE_NAME_BLOCK_HEIGHT, COLOR_TURQUOISE);

  _mqttInit();
  
  // Set up the data collection timer
    timeToSendData = true;

    // Use 1st timer of 4 (counted from zero).
    // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
    // info).
    dataCollectionTimer = timerBegin(0, 80, true);
  
    // Attach callback function to our timer.
    timerAttachInterrupt(dataCollectionTimer, &dataCollectionTimerCallback, true);
  
    // Set alarm to call onTimer function every second (value in microseconds).
    // Repeat the alarm (third parameter)
    timerAlarmWrite(dataCollectionTimer, (getDataPublishInterval() * 1000000), true);
  
    // Start an alarm
    timerAlarmEnable(dataCollectionTimer);

#if 0
  // Set up the WiFi re-connect timer
    wifiAttemptCount = 0;
    timeToConnectToWifi = true;

    // Use 2nd timer of 4 (counted from zero).
    // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
    // info).
    wifiConnectTimer = timerBegin(1, 80, true);
  
    // Attach callback function to our timer.
    timerAttachInterrupt(wifiConnectTimer, &wifiConnectCallback, true);
  
    // Set alarm to call onTimer function every 30 seconds (value in microseconds).
    // Repeat the alarm (third parameter)
    timerAlarmWrite(wifiConnectTimer, 30000000, true);

    Serial.printf("Set up the WiFi re-connect timer.\n");
    delay(1000);
#endif

    timerEnd(statusLedTimer);
    digitalWrite( STATUS_LED, LOW);
    g_statusLedColour = neoStatusLed.Color(0, 255, 0); // GREEN
    neoStatusLed.setPixelColor(0, g_statusLedColour); 
    neoStatusLed.show();
    neoStatusLed.setBrightness(g_statusLedBrightness);
    neoStatusLed.show();
    Serial.printf("LED should be green with brightness [%d]\n", g_statusLedBrightness);
    displayDeviceName(getDeviceName());
    delay(5000);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (g_mqttConnected) {

      mqttClient.loop();

      readData();
      
      uiMgr.sync();

      // configMgr.sync();

      if (timeToSendData) {
        sendData();
        timeToSendData = false;
      }

      boolean motionStateChanged = readMotionData();
      if (motionStateChanged) {
        sendMotionData();
      }

    } else {
      mqttReconnectCount++;
      g_mqttConnected = mqttConnectLocal(getMqttBroker(), mqttCallback, g_deviceName);
    }
  } else {
    Serial.println("WiFi not connected. Re-trying connection");
    wifiReconnectCount++;
    connectToWifi();
  }
}

//-------------------------------------------------------------------------------

void displayConfigMode(char* apName, IPAddress accessPointIp) {
//    tft.fillRectangle(TIME_BLOCK_X, (TIME_BLOCK_Y - 30), LCD_WIDTH, 34, COLOR_BLACK);
//    display.setTextSize(1);
//    display.setCursor(TIME_BLOCK_X,TIME_BLOCK_Y - 40);
//    display.setTextColor(YELLOW);
////    display.println("Config Mode");
//    display.println("Connect to WiFi:");
//    display.setCursor(TIME_BLOCK_X,TIME_BLOCK_Y - 30);
//    display.setTextColor(WHITE);
//    display.println(apName);
//    display.setTextColor(YELLOW);
//    display.setCursor(TIME_BLOCK_X,TIME_BLOCK_Y - 15);
//    display.println("Then go to: ");
//    display.setCursor(TIME_BLOCK_X,TIME_BLOCK_Y - 5);
//    display.setTextColor(WHITE);
//    display.println(accessPointIp);    
//    display.setTextColor(YELLOW);
}

void displayWifiConnecting(char* ssid) {
  tft.clear();
  tft.setFont(Terminal6x8);
  String myString = "Connecting to: " + String(ssid);
  tft.drawText(TIME_BLOCK_X, TIME_BLOCK_Y - 20, myString, COLOR_YELLOW);
}

void displayWifiConnected(char* ssid) {
  tft.clear();
//    display.setTextSize(1);
//    display.setCursor(TIME_BLOCK_X,TIME_BLOCK_Y - 30);
//    display.setTextColor(GREEN);
//    display.println("Connected to: ");
//    display.setTextColor(YELLOW);
//    display.println(ssid);
//    display.println("");
//    display.println(WiFi.localIP());
}

void displayMotionIcon() {
  
}

void removeMotionIcon() {

}


//-------------------------------------------------------------------------------

void readData() {
//  Serial.printf("----------------------\n");
  readTemp();
#ifndef HEADLESS
  if (currentTemp != oldTemp) {
    displayTemp(false);
  }
#endif

  readHumidity();
#ifndef HEADLESS
  if (currentHumidity != oldHumidity) {
    displayHumidity(false);
  }
#endif
  
  readMotionData();
  readLightData();
  
  if (WiFi.status() == WL_CONNECTED) {
    readDateTime();
#ifndef HEADLESS
    displayDateTime(false);
#endif
  }

  timeToSendData = false;
}

void readTemp() {
  oldTemp = currentTemp;
  currentTemp = dht.readTemperature() + g_temperatureCalibrationValue;
//  Serial.print("Temperature: ");
//  Serial.print(currentTemp);
//  Serial.println(" *C ");

//  Serial.printf("Temperature: %d  *C \n", currentTemp);
}

void readHumidity() {
  oldHumidity = currentHumidity;
  currentHumidity = dht.readHumidity() + g_humidityCalibrationValue;
//  Serial.print("Humidity: ");
//  Serial.print(currentHumidity);
//  Serial.println(" %\t");
  
//  Serial.printf("Humidity: %d %\n", currentHumidity);
}

void readDateTime() {
    oldDate = date;
    oldTime = currentTime;
    
    date = "";  // clear the variables
    currentTime = "";

    // update the NTP client and get the UNIX UTC timestamp 
    timeClient.update();
    unsigned long epochTime =  timeClient.getEpochTime();

    // convert received time stamp to time_t object
    time_t local, utc;
    utc = epochTime;

    // Then convert the UTC UNIX timestamp to local time
    TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -300};  //UTC - 5 hours - change this as needed
    TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -360};   //UTC - 6 hours - change this as needed
    Timezone usEastern(usEDT, usEST);
    local = usEastern.toLocal(utc);

    // now format the Time variables into strings with proper names for month, day etc
    date += days[weekday(local)-1];
    date += ", ";
    date += months[month(local)-1];
    date += " ";
    date += day(local);
//    date += ", ";
//    date += year(local);

    // format the time to 12-hour format with AM/PM and no seconds
    currentTime += hourFormat12(local);
    currentTime += ":";
    if(minute(local) < 10)  // add a zero if minute is under 10
      currentTime += "0";
    currentTime += minute(local);
    // currentTime += " ";
    // currentTime += ampm[isPM(local)];

}

void sendData() {
  if (mqttClient.connected()) {
    // Publish
    char topic[256];
    char value[8];

    sprintf(topic, "%s/%s/temperature", MQTT_TOPIC_ROOT, g_deviceName);
    sprintf(value, "%.1f", currentTemp);
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
    sprintf(topic, "%s/%s/humidity", MQTT_TOPIC_ROOT, g_deviceName);
    sprintf(value, "%.1f", currentHumidity);
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
    sendMotionData();
    sendLightData();
  } else {
    Serial.println("MQTT broker not connected. Can't publish data");
    g_mqttConnected = false;
  }
}

void readLightData() {
  // Light data is read at the time of publishing.
  // Serial.printf("Light: %d\n", analogRead(LIGHT_SENSOR_PIN));
}

void sendLightData() {
  if (mqttClient.connected()) {
    char topic[256];
    char value[8];
    sprintf(topic, "%s/%s/light", MQTT_TOPIC_ROOT, g_deviceName);
    sprintf(value, "%d", analogRead(LIGHT_SENSOR_PIN));
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
  } else {
    Serial.println("MQTT broker not connected. Can't publish light data");
    g_mqttConnected = false;
  }
}

void sendStatusLedBrightness(int value) {
  if (mqttClient.connected()) {
    char topic[256];
    char value[8];
    sprintf(topic, "%s/%s/%s", MQTT_TOPIC_ROOT, g_deviceName, SUB_TOPIC_LED_BRIGHTNESS);
    sprintf(value, "%d", value);
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
  } else {
    Serial.println("MQTT broker not connected. Can't publish status LED brightness data");
    g_mqttConnected = false;
  }
}

bool readMotionData() {
  bool motionStateChanged = false;
  
  if(digitalRead(MOTION_SENSOR_PIN) == HIGH ){
    motionDetectedTime = millis();
    if (!currentMotionDetectedState) {
      currentMotionDetectedState = true;
      motionStateChanged = true;
//      digitalWrite(STATUS_LED, HIGH);
      displayMotionIcon();
      Serial.println("Motion detected");      
    }
  } else {
    if( currentMotionDetectedState == true && (millis() -  motionDetectedTime) > noMotionDelay ) {
      currentMotionDetectedState = false;
      motionStateChanged = true;
//      digitalWrite(STATUS_LED, LOW);
      removeMotionIcon();
      Serial.println("No motion detected");
    }
  }
  return motionStateChanged;
}

void sendMotionData() {
  if (mqttClient.connected()) {
    // Publish
    char topic[256];
    char value[8];
    sprintf(topic, "%s/%s/motion", MQTT_TOPIC_ROOT, g_deviceName);
    sprintf(value, "%d", (currentMotionDetectedState ? 1 : 0)); // these relate to colours in IOT-ON-OFF app
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
//    Serial.printf("Motion: %d\n", currentMotionDetectedState);
  } else {
    Serial.println("MQTT broker not connected. Can't publish motion data");
    g_mqttConnected = false;
  }
}

void sendTemperatureCalibrationValue() {
  if (mqttClient.connected()) {
    char topic[256];
    char value[6];
    sprintf(topic, "%s/%s/%s", MQTT_TOPIC_ROOT, g_deviceName, SUB_TOPIC_TEMP_CALIB);
    sprintf(value, "%.1f", g_temperatureCalibrationValue);
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
  } else {
    Serial.println("MQTT broker not connected. Can't publish temperature calibration data");
    g_mqttConnected = false;
  }
}

void sendHumidityCalibrationValue() {
  if (mqttClient.connected()) {
    char topic[256];
    char value[6];
    sprintf(topic, "%s/%s/%s", MQTT_TOPIC_ROOT, g_deviceName, SUB_TOPIC_HUMIDITY_CALIB);
    sprintf(value, "%.1f", g_humidityCalibrationValue);
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
  } else {
    Serial.println("MQTT broker not connected. Can't publish humidity calibration data");
    g_mqttConnected = false;
  }
}

void displayTemp(boolean force){
  char currentTempChar[8];
  char oldTempChar[8];
  
  tft.setGFXFont(&FreeSans12pt7b);

  if (force || oldTemp != currentTemp) {
    // Delete the old value
    sprintf(oldTempChar, "%.1fC\0", oldTemp);
    String oltTempString = String(oldTempChar);
    tft.drawGFXText(TEMP_BLOCK_X, TEMP_BLOCK_Y + TEMP_BLOCK_HEIGHT, oltTempString, COLOR_BLACK);
  
    // Draw the new value
    sprintf(currentTempChar, "%.1fC\0", currentTemp);
    String currentTempString = String(currentTempChar);
    tft.drawGFXText(TEMP_BLOCK_X, TEMP_BLOCK_Y + TEMP_BLOCK_HEIGHT, currentTempString, COLOR_BLUE);
  }
}

void displayHumidity(boolean force){
  char currentHumidityChar[8];
  char oldHumidityChar[8];
  
  tft.setGFXFont(&FreeSans12pt7b);
  if ( force || oldHumidity != currentHumidity) {
    sprintf(oldHumidityChar, "%.1f%%\0", oldHumidity);
    String oldHumidityString = String(oldHumidityChar);
    tft.drawGFXText(HUMIDITY_BLOCK_X, HUMIDITY_BLOCK_Y + HUMIDITY_BLOCK_HEIGHT, oldHumidityString, COLOR_BLACK);
    
    sprintf(currentHumidityChar, "%.1f%%\0", currentHumidity);
    String currentHumidityString = String(currentHumidityChar);
    tft.drawGFXText(HUMIDITY_BLOCK_X, HUMIDITY_BLOCK_Y + HUMIDITY_BLOCK_HEIGHT, currentHumidityString, COLOR_BLUE);
  }
}

void displayDateTime(boolean force){
  displayTime(force);
  displayDate(force);
}

char* string2char(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
}

void displayTime(boolean force) {
  if (force || oldTime != currentTime) {
//    Serial.printf("displayTime: Updating date to: %s\n", t);
//    Serial.printf("Filling rect: \n");
//    Serial.printf("X = TIME_BLOCK_X = %d\n", TIME_BLOCK_X);
//    Serial.printf("Y = TIME_BLOCK_Y = %d\n", TIME_BLOCK_Y);
//    Serial.printf("X' = LCD_WIDTH - TIME_BLOCK_X = %d - %d = %d\n", LCD_WIDTH, TIME_BLOCK_X, (LCD_WIDTH - TIME_BLOCK_X));
//    Serial.printf("Y' = TIME_BLOCK_Y + TIME_BLOCK_HEIGHT = %d + %d = %d\n", TIME_BLOCK_Y, TIME_BLOCK_HEIGHT, TIME_BLOCK_Y + TIME_BLOCK_HEIGHT);
    tft.fillRectangle(TIME_BLOCK_X, TIME_BLOCK_Y, LCD_WIDTH - TIME_BLOCK_X, TIME_BLOCK_Y + TIME_BLOCK_HEIGHT, COLOR_BLACK);
    tft.setGFXFont(&FreeSansBold24pt7b);
    int xPos = getXCentered(currentTime, TIME_BLOCK_WIDTH) + TIME_BLOCK_X;
    tft.drawGFXText(xPos, TIME_BLOCK_Y + TIME_BLOCK_HEIGHT, currentTime, COLOR_RED);
  }
}

void displayDate(boolean force){
  int val=0;
  if (force || oldDate != date) {
//    Serial.printf("displayDate: Updating date to: %s\n", date);
//    Serial.printf("Filling rect: \n");
//    Serial.printf("X = DATE_BLOCK_X = %d\n", DATE_BLOCK_X);
//    Serial.printf("Y = DATE_BLOCK_Y = %d\n", DATE_BLOCK_Y);
//    Serial.printf("X' = LCD_WIDTH - DATE_BLOCK_X = %d - %d = %d\n", LCD_WIDTH, DATE_BLOCK_X, (LCD_WIDTH - DATE_BLOCK_X));
//    Serial.printf("Y' = DATE_BLOCK_Y + DATE_BLOCK_HEIGHT = %d + %d = %d\n", DATE_BLOCK_Y, DATE_BLOCK_HEIGHT, DATE_BLOCK_Y + DATE_BLOCK_HEIGHT);
    tft.fillRectangle(DATE_BLOCK_X, DATE_BLOCK_Y, LCD_WIDTH - DATE_BLOCK_X, DATE_BLOCK_Y + DATE_BLOCK_HEIGHT, COLOR_BLACK);
    tft.setGFXFont(&FreeSans12pt7b);
    int xPos = getXCentered(date, DATE_BLOCK_WIDTH) + DATE_BLOCK_X;
    tft.drawGFXText(xPos, DATE_BLOCK_Y + DATE_BLOCK_HEIGHT - 5, date, COLOR_YELLOW);
  }
}

void displayWifi(boolean force){
}

void displayAll(boolean force) {
  displayWifi(force);
  displayDate(force);
  displayTime(force);
  displayTemp(force);
  displayHumidity(force);  
}

void criticalCodeStart() {
//  Serial.println("Crit section start..............");
  noInterrupts();
  timerEnd(dataCollectionTimer);
}

void criticalCodeEnd() {
  interrupts();
  timerAlarmEnable(dataCollectionTimer);
//  Serial.println("Crit section enc ^^^^^^^^^^^^^^^");
}


boolean mqttConnectLocal(char* broker, void (*callback)(char*, uint8_t*, unsigned int), const char* deviceName) {
  boolean retVal = false;
  
  if (broker) {
    Serial.printf("mqttConnect: Connecting to mqttBroker = [%s]\n", broker);  
//  delay(2000);
    mqttClient.setServer(broker, 1883);
    mqttClient.setCallback(mqttCallback);    
//    Serial.printf("Attempting to connect to MQTT Broker [%s]\n", deviceName);
//    delay(2000);
    if (mqttClient.connect(deviceName)) {
      Serial.printf("Connected to MQTT broker [%s]\n", broker);
      mqttClient.subscribe(g_mqttTopicDevice);
//      boolean result = mqttClient.subscribe("inTopic");
//      if (!result) {
//        Serial.println("ERROR: Failed to subscribe to topic inTopic");
//      }
      Serial.printf("Subscribed to topic: [%s]\n", g_mqttTopicDevice);
//      Serial.printf("Subscribed to topic: [%s]\n","inTopic");
    } else {
      Serial.printf("mqttConnect: ERROR: Failed to connect to MQTT broker [%s]\n", broker);
    }

    if (mqttClient.connected()) {
      retVal = true;
      Serial.println("MQTT connection verified");
    } else {
      Serial.println("Unable to verify MQTT connectio");
    }
  } else {
    Serial.println("mqttConnect: ERROR: Unable to connect. broker = NULL");
  }
  return retVal;
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("outTopic", "hello world");
      // ... and resubscribe
      mqttClient.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  char* payloadStr = (char*)malloc((length+1) * sizeof(char));
  memcpy(payloadStr, payload, length);
  payloadStr[length] = '\0';

  char* command;
  char* token;
  char* lastToken;

  token = strtok((char*)topic, "/");
  
  while (NULL != token) {
    lastToken = token;
    token = strtok(NULL, "/");
  }
  command = lastToken;

  // Brightness -------------------------
  if (0 == strcmp(command, SUB_TOPIC_BRIGHTNESS)) {
    g_displayBacklightBrightness = (int)atoi(payloadStr);    
  } else
  // Backlight -------------------------
  if (0 == strcmp(command, SUB_TOPIC_BACKLIGHT)) {
    g_displayBacklightState = (int)atoi(payloadStr);
  } else
  // flip -------------------------
  if (0 == strcmp(command, SUB_TOPIC_FLIP)) {
    Serial.println("flipping the display");
    if (g_orientation == DISPLAY_ORIENTATION_0) {
      g_orientation = DISPLAY_ORIENTATION_180;
    } else {
      g_orientation = DISPLAY_ORIENTATION_0;
    }
  } else
  // temp calibration -------------------------
  if (0 == strcmp(command, SUB_TOPIC_TEMP_CALIB)) {
    float value = (float)atof(payloadStr);
    envMonConfig.setTemperatureCalibrationValue(value);
  } else
  // humidity calibration -------------------------
  if (0 == strcmp(command, SUB_TOPIC_HUMIDITY_CALIB)) {
    Serial.println("Adjusting humidity calibration");
    float value = (float)atof(payloadStr);
    envMonConfig.setHumidityCalibrationValue(value);
  } else
  // Neo Status LED brightness -------------------------
  if (0 == strcmp(command, SUB_TOPIC_LED_BRIGHTNESS)) {
//    Serial.println("Adjusting led brightness");
    int value = (int)atoi(payloadStr);
    if (value >= 0 && value <= 255) {
      g_statusLedBrightness = value;
    } else {
      Serial.printf("Invalid value for orientation: %d\n", value);
    }
  } else 
  // Neo Status LED colour -------------------------
  if (0 == strcmp(command, SUB_TOPIC_LED_COLOUR)) {
    Serial.println("Setting led colour");
//    neoStatusLed.setPixelColor(1, g_statusLedColour);
//    neoStatusLed.show();
  } else {
    Serial.printf("#### unhandled command [%s]####\n", command);
  }
    
}

