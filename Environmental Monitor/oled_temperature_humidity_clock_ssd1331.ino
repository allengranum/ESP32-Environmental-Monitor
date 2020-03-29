#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <WifiUDP.h>
#include <DHT.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>
#include "esp_onboarding.h"
#include "esp_config.h"
#include "esp_mqtt.h"
//#include "bitmaps.h"
#include <PubSubClient.h>
#include <Timezone.h>
#include <TFT_22_ILI9225.h>

#include <../fonts/FreeSans12pt7b.h>
#include <../fonts/FreeSans24pt7b.h>
#include <../fonts/FreeSansBold24pt7b.h>
#include <../fonts/FreeSans9pt7b.h>

//#define HEADLESS 1

/******************************************
 * ESP32 pin assignments                  *
 *                                        */
/* INPUT PINS                             */
#define DHT_SENSOR_PIN       21
#define MOTION_SENSOR_PIN    12
#define LIGHT_SENSOR_PIN     36
#define GAS_SENSOR_PIN       26
#define SOUND_SENSOR_PIN      4
#define CONFIG_BUTTON        23
#define DELETE_CONFIG_BUTTON 22

/* SPI PINS                               */
#define rst                  15
#define dc                   19
#define mosi                 18
#define sclk                  5
#define cs                   32

/* OUTPUT PINS                            */
#define STATUS_LED           13
#define NEO_STATUS_LED_PIN   14
#define NEO_NIGHTLIGHT_PIN   27
#define BUZZER_PIN           25
#define TFT_RS               33
/******************************************/


#define FAST_FLASH_uS 500000
#define DATA_READ_INTERVAL_SECONDS 30

//
// NeoPixel Status LED
#define LED_COUNT 1
#define DEFAULT_STATUS_LED_BRIGHTNESS  5
Adafruit_NeoPixel neoStatusLed(LED_COUNT, NEO_STATUS_LED_PIN, NEO_GRB + NEO_KHZ800);

//
// NTP & time stuff
//
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

//
// DHT temperature stuff
#define DHTTYPE DHT22

#define DATA_PUBLISH_INTERVAL_KEY "DATA_PUBLISH_INTERVAL_SECONDS"
#define DEFAULT_DATA_PUBLISH_INTERVAL_SECONDS 5

float g_temperatureCalibrationValue = 0;
float g_humidityCalibrationValue = 0;
DHT dht(DHT_SENSOR_PIN, DHTTYPE);

#define TFT_LED 0
#define TFT_RS 19
#define TFT_BRIGHTNESS 150
#define ORIENTATION_0   1
#define ORIENTATION_90  2
#define ORIENTATION_180 3
#define ORIENTATION_270 4

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
//TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
// Use software SPI (slower)
TFT_22_ILI9225 tft = TFT_22_ILI9225(rst, TFT_RS, cs, mosi, sclk, TFT_LED, TFT_BRIGHTNESS);

#define LCD_HEIGHT 176
#define LCD_WIDTH  220

float currentTemp, oldTemp;
float currentHumidity, oldHumidity;

String date, oldDate;
String currentTime, oldTime;
const char * days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;

#define MAX_WIFI_CONNECT_ATTEMPTS 3
#define WIFI_CONNECT_DELAY_MS 500

#define TEMP_BLOCK_X       5
#define TEMP_BLOCK_Y      0
#define TEMP_BLOCK_HEIGHT 25
#define TEMP_BLOCK_WIDTH  70

#define HUMIDITY_BLOCK_X     150
#define HUMIDITY_BLOCK_Y      0
#define HUMIDITY_BLOCK_HEIGHT 25
#define HUMIDITY_BLOCK_WIDTH  70

#define TIME_BLOCK_X       0
#define TIME_BLOCK_Y      50
#define TIME_BLOCK_HEIGHT 40
#define TIME_BLOCK_WIDTH 220

#define MOTION_ICON_BOX_X 
#define MOTION_ICON_BOX_Y

#define DATE_BLOCK_X        2
#define DATE_BLOCK_Y      100
#define DATE_BLOCK_HEIGHT  25
#define DATE_BLOCK_WIDTH  220

#define DEVICE_NAME_BLOCK_X       60
#define DEVICE_NAME_BLOCK_Y      151
#define DEVICE_NAME_BLOCK_HEIGHT  25
#define DEVICE_NAME_BLOCK_WIDTH  100


hw_timer_t * dataCollectionTimer = NULL;
hw_timer_t * wifiConnectTimer = NULL;
hw_timer_t * statusLedTimer = NULL;


bool timeToCollectData;
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
int loopCount = 0;
int g_orientation = 3;
uint32_t g_statusLedColour;
static int g_statusLedBrightness;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Prototypes
void connectToWifi(void);
void readData(void);
void sendData(void);
void sendMotionData(void);
void reconnect(void);
void displayAll(void);


void dataCollectionTimerCallback() {
  timeToCollectData = true;
}

void statusLedTimerCallback() {
  static boolean neoStatusLedOn;
//  Serial.printf("statusLedTimerCallback: neoStatusLedOn = [%d]\n", neoStatusLedOn);
//  Serial.printf("statusLedTimerCallback: millis = %d\n", millis());
  
//  if(neoStatusLedOn) {
//    Serial.println("turn status led OFF");
//    neoStatusLed.setBrightness(0);
//    neoStatusLed.setPixelColor(0, neoStatusLed.Color(0,   0,   0));
//    neoStatusLed.show();
//    neoStatusLedOn = false;
//  } else {
//    Serial.println("turn status led ON");
//    neoStatusLed.setBrightness(g_statusLedBrightness);
//    neoStatusLed.setPixelColor(0, g_statusLedColour);
//    neoStatusLed.show();
//    neoStatusLedOn = true;
//  }
}

void wifiConnectCallback() {
      timeToConnectToWifi = true;
}

void configButtonPressHandler() {
  Serial.println("CONFIG_BUTTON pressed!!!");
  //deleteConfigData();
}

float getTemperatureCalibrationValue() {
  return getCustomValueFloat(TEMP_CALIB_KEY);
}

boolean saveTemperatureCalibrationValue(char* value) {
  saveKeyValue( TEMP_CALIB_KEY, value, true);
  sendTemperatureCalibrationValue();
}

int getHumidityCalibrationValue() {
    return getCustomValueInt(HUMIDITY_CALIB_KEY);    
}

boolean saveHumidityCalibrationValue(char* value) {
  saveKeyValue( HUMIDITY_CALIB_KEY, value, true);
  sendHumidityCalibrationValue();
}

boolean saveStatusLedBrightness(int value) {
  char valueStr[4];
  sprintf(valueStr, "%d", value);
  saveKeyValue( STATUS_LED_BTIGHTNESS_KEY, valueStr, true);
}

int getStatusLedBrightness() {
    int value;
    value = getCustomValueInt(STATUS_LED_BTIGHTNESS_KEY);
    Serial.printf("getStatusLedBrightness: Value = %d\n",value);
    if (value == 0) {
      Serial.printf("getStatusLedBrightness: Warning: No saved brightness.\n");
      Serial.printf("getStatusLedBrightness: Saving default brightness value = [%d]\n", DEFAULT_STATUS_LED_BRIGHTNESS);
      saveStatusLedBrightness(DEFAULT_STATUS_LED_BRIGHTNESS);
      sendStatusLedBrightness(DEFAULT_STATUS_LED_BRIGHTNESS);
      return DEFAULT_STATUS_LED_BRIGHTNESS;
    } else {
      return value;
    }
}

void setStatusLedBrightness(int value) {  
  Serial.printf("setStatusLedBrightness: current = [%d], new = [%d]\n", g_statusLedBrightness,  value);
  if (value != g_statusLedBrightness) {
    Serial.printf("setStatusLedBrightness: saveStatusLedBrightness\n");
    neoStatusLed.setBrightness(g_statusLedBrightness);
    saveStatusLedBrightness(value);
    //sendStatusLedBrightness(value);
  }
  Serial.printf("setStatusLedBrightness: End\n");
}

int getDataPublishInterval() {
    int value = getCustomValueInt(DATA_PUBLISH_INTERVAL_KEY);
    if (value == 0) {
      value = DEFAULT_DATA_PUBLISH_INTERVAL_SECONDS;
    }
    Serial.printf("getDataPublishInterval = [%d]\n", value);
    return value;
}

char* getMqttBroker() {
  return getCustomValue(MQTT_BROKER_KEY);
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
//  Serial.printf("payloadStr = [%s]\n", payloadStr);

  char* command;
  char* token;
  char* lastToken;

  token = strtok((char*)topic, "/");
  
  while (NULL != token) {
//    Serial.printf("current token = [%s]\n", token);
    lastToken = token;
    token = strtok(NULL, "/");
  }
  command = lastToken;

  // Brightness -------------------------
//  Serial.printf("received command: [%s]\n", command);
  if (0 == strcmp(command, SUB_TOPIC_BRIGHTNESS)) {
    int value = (int)atoi(payloadStr);
    Serial.printf("Adjusting brightness to: %d\n", value);
    tft.setBacklightBrightness(value);
  } else
  // Backlight -------------------------
  if (0 == strcmp(command, SUB_TOPIC_BACKLIGHT)) {
    int value = (int)atoi(payloadStr);
//    Serial.printf("Turning backlight %s", value = 0 ? "off" "on");
    if (0 == value) {
      Serial.println("backlight setting to false");
      tft.setBacklight(false);
    } else {
      Serial.println("backlight setting to true");
      tft.setBacklight(false);
    }
    tft.clear();
    displayAll(true);
  } else
  // orientation -------------------------
  if (0 == strcmp(command, SUB_TOPIC_ORIENTATION)) {
    Serial.println("Adjusting orientation");
    int value = (int)atoi(payloadStr);
    if (value > 0 && value < 5) {
      tft.setOrientation(value);
      tft.clear();
      displayAll(true);
    } else {
      Serial.printf("Invalid value for orientation: %d\n", value);
    }
  } else
  // flip -------------------------
  if (0 == strcmp(command, SUB_TOPIC_FLIP)) {
    Serial.println("flipping the display");
    if (g_orientation == ORIENTATION_0) {
      g_orientation = ORIENTATION_180;
    } else {
      g_orientation = ORIENTATION_0;
    }
    tft.setOrientation(g_orientation);
    tft.clear();
    displayAll(true);
      mqttClient.publish(topic, "clearing if retained", false);
  } else
  // temp calibration -------------------------
  if (0 == strcmp(command, SUB_TOPIC_TEMP_CALIB)) {
    Serial.println("Adjusting temperature calibration");
    float value = (float)atof(payloadStr);
    g_temperatureCalibrationValue = value;
    readTemp();
#ifndef HEADLESS
    if (currentTemp != oldTemp) {
      displayTemp(false);
    }
#endif
    saveTemperatureCalibrationValue(payloadStr);
  } else
  // humidity calibration -------------------------
  if (0 == strcmp(command, SUB_TOPIC_HUMIDITY_CALIB)) {
    Serial.println("Adjusting humidity calibration");
    float value = (float)atof(payloadStr);
    g_humidityCalibrationValue = value;
    readHumidity();
#ifndef HEADLESS
    if (currentTemp != oldTemp) {
      displayHumidity(false);
    }
#endif
    saveHumidityCalibrationValue(payloadStr);
  } else
  // Neo Status LED brightness -------------------------
  if (0 == strcmp(command, SUB_TOPIC_LED_BRIGHTNESS)) {
//    Serial.println("Adjusting led brightness");
    int value = (int)atoi(payloadStr);
    if (value >= 0 && value <= 255) {
      // setStatusLedBrightness(value);
      neoStatusLed.setBrightness(value);
      neoStatusLed.setPixelColor(0, g_statusLedColour);
      neoStatusLed.show();
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

void _configInit() {
  g_statusLedBrightness = getStatusLedBrightness();
  g_temperatureCalibrationValue = getTemperatureCalibrationValue();
  g_humidityCalibrationValue = getHumidityCalibrationValue();
}

void _statusLedInit() {
  neoStatusLed.begin();
  neoStatusLed.clear();
  neoStatusLed.show();
  g_statusLedColour = neoStatusLed.Color(255,   0,   0);
  neoStatusLed.setPixelColor(0, g_statusLedColour);
  neoStatusLed.setBrightness(g_statusLedBrightness);
  neoStatusLed.show();
}

void _displayInit() {
  tft.begin();
  tft.setOrientation(3);
  tft.clear();
  tft.setBackgroundColor(COLOR_BLACK);
  tft.setBacklightBrightness(100);
  displayDeviceName(getDeviceName());
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
    deleteConfigData();
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

void _mqttInit() {
  g_deviceName = getDeviceName();
  g_mqttTopicRoot = getMqttTopicRoot();
  
  // Build the root MQTT topic for the device that  it will
  // subscribe to for receiving device commands
  sprintf(g_mqttTopicDevice, "/%s/cmd/+", g_deviceName);
  Serial.printf("g_mqttTopicDevice = [%s]\n", g_mqttTopicDevice);
  
  sprintf(g_topic_brightness, "%s/%s/%s", MQTT_TOPIC_ROOT, g_deviceName, SUB_TOPIC_BRIGHTNESS);
  Serial.printf("g_topic_brightness = [%s]\n", g_topic_brightness);

  sprintf(g_topic_orientation, "%s/%s/%s", MQTT_TOPIC_ROOT, g_deviceName, SUB_TOPIC_ORIENTATION);
  Serial.printf("g_topic_orientation = [%s]\n", g_topic_orientation);

  sprintf(g_topic_temp_calib, "%s/%s/%s", MQTT_TOPIC_ROOT, g_deviceName, SUB_TOPIC_TEMP_CALIB);
  Serial.printf("g_topic_temp_calib = [%s]\n", g_topic_temp_calib);

  sprintf(g_topic_humidity_calib, "%s/%s/%s", MQTT_TOPIC_ROOT, g_deviceName, SUB_TOPIC_HUMIDITY_CALIB);
  Serial.printf("g_topic_humidity_calib = [%s]\n", g_topic_humidity_calib);

  mqttClient.setServer(getMqttBroker(), 1883);
  mqttClient.setCallback(mqttCallback);
  g_mqttConnected = mqttConnectLocal(getMqttBroker(), mqttCallback, g_deviceName);
  if (g_mqttConnected) {
    char topic[256];
    char value[2];

    sprintf(topic, "%s/%s/%s", MQTT_TOPIC_ROOT, g_deviceName, SUB_TOPIC_CONNECTED);
    sprintf(value, "1");
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
    sprintf(topic, "%s/%s/%s", MQTT_TOPIC_ROOT, g_deviceName, SUB_TOPIC_TEMP_CALIB);
    sprintf(value, "", g_temperatureCalibrationValue);
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);





    g_statusLedBrightness = getStatusLedBrightness();
  g_temperatureCalibrationValue = getTemperatureCalibrationValue();
  g_humidityCalibrationValue = getHumidityCalibrationValue();
  }
}
void setup() {

  Serial.begin(9600);

  _configInit();

  _statusLedInit();

  _gpioInit();

  _displayInit();

  _buttonsInit();

  _motionSensorInit();

  _flashStatusLed(100, 100);

  delay(1000); // this is magical. if you take it out, things go bad
  dumpConfig();

  connectToWifi();
  
  // tft.fillRectangle(TEMP_BLOCK_X, TEMP_BLOCK_Y, TEMP_BLOCK_X + TEMP_BLOCK_WIDTH, TEMP_BLOCK_Y + TEMP_BLOCK_HEIGHT, COLOR_ORANGE);
  // tft.fillRectangle(HUMIDITY_BLOCK_X, HUMIDITY_BLOCK_Y, HUMIDITY_BLOCK_X + HUMIDITY_BLOCK_WIDTH, HUMIDITY_BLOCK_Y + HUMIDITY_BLOCK_HEIGHT, COLOR_SILVER);
  // tft.fillRectangle(TIME_BLOCK_X, TIME_BLOCK_Y, TIME_BLOCK_X + TIME_BLOCK_WIDTH, TIME_BLOCK_Y+TIME_BLOCK_HEIGHT, COLOR_SKYBLUE);
  // tft.fillRectangle(DATE_BLOCK_X, DATE_BLOCK_Y, DATE_BLOCK_X + DATE_BLOCK_WIDTH, DATE_BLOCK_Y+DATE_BLOCK_HEIGHT, COLOR_GREEN);
  // tft.fillRectangle(DEVICE_NAME_BLOCK_X, DEVICE_NAME_BLOCK_Y, DEVICE_NAME_BLOCK_X + DEVICE_NAME_BLOCK_WIDTH, DEVICE_NAME_BLOCK_Y + DEVICE_NAME_BLOCK_HEIGHT, COLOR_TURQUOISE);

  _mqttInit();

  dht.begin();

  Wire.begin();
  
  // Set up the data collection timer
    timeToCollectData = true;

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
      if (timeToCollectData) {
        readData();
        sendData();
        timeToCollectData = false;
      }

      if (readMotionData()) {
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

void connectToWifi() {
//  timerEnd(wifiConnectTimer);
  
  // Get the list of known wifi networks from the config file
  WifiInfo* knownNetworks = getSsids();
  
  if (knownNetworks->numberOfSSIDsFound > 0) {
    Serial.printf("connectToWifi: %d known networks found in config file\n", knownNetworks->numberOfSSIDsFound);
    delay(500);
    for (int i=0;i<knownNetworks->numberOfSSIDsFound;i++) {
      Serial.printf("    %d: [%s]\n", i, knownNetworks->ssid[i]);
    }

    // Get a list of available wifi networks
    WifiInfo* availableNetworks = scanForAPs();
        
    char* ssid = NULL;
    char* password = NULL;
    int index=0;
    
    while( index < availableNetworks->numberOfSSIDsFound && ssid == NULL ) {
      char* currentSsid = availableNetworks->ssid[index];
      Serial.printf("Checking if I know about ssid [%s]\n", currentSsid);
      // See if it exists in the known list
      for (int i=0;i<knownNetworks->numberOfSSIDsFound;i++) {
        if (0 == strcmp(currentSsid, knownNetworks->ssid[i])) {
          ssid = knownNetworks->ssid[i];
        }
      }

      // if yes, get the associated passsword and attempt to connect
      if (NULL != ssid) {
        Serial.printf("connectToWifi: Attempting to connect to SSID: [%s]\n", ssid);
        password = getWifiPassword(ssid);
        Serial.printf("connectToWifi: Using password: [%s]\n", password);

        displayWifiConnecting(ssid);
                
        WiFi.begin(ssid, password);
        
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }

        displayWifiConnected(ssid);
        
        Serial.printf("WiFi connected\n");
        Serial.printf("IP address: \n");
        Serial.println(WiFi.localIP());
      } else {
          Serial.printf("I don't know any of the available WiFi networks.\n");
      }
      index++;
      // if no, get the next available network from the top of the available list and try again    
    }
    
    // Free up memory
    free(availableNetworks->ssid);
    free(availableNetworks);
    free(knownNetworks->ssid);

    if (WiFi.status() != WL_CONNECTED) {
      performOnboarding();
    }
  } else {
    Serial.println("connectToWifi: need to perform onboarding");
    delay(500);
    performOnboarding();
  }   
}

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

  timeToCollectData = false;
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
//------------------------------------------------------------------------------
int getXCentered(String string, int width) {
  int16_t x1, y1, xPos;
  int16_t w, h;

//  Serial.print("getXCentered: string=[");
//  Serial.print(string);
//  Serial.println("]");
//  Serial.print("String length = ");
//  Serial.println(string.length());
  // Determine coordinates to center the time
  tft.getGFXTextExtent(string, x1, y1, &w, &h);
//  Serial.print("width of string = ");
//  Serial.println(w);
  xPos = (width-w)/2;
//  Serial.print("calculated xPos = ");
//  Serial.println(xPos);
//  Serial.println("");
//  Serial.println("");  
  return xPos;
}

int getXRight(float val) {
  int16_t x1, y1, xPos;
  int16_t w, h;
  char buff[10];

  dtostrf(val, 3, 0, buff);  //4 is mininum width, 6 is precision
  // Determine coordinates to center the time
  tft.getGFXTextExtent(buff, x1, y1, &w, &h);
  xPos = (LCD_HEIGHT-w);
  tft.getGFXTextExtent("%", x1, y1, &w, &h);
  xPos-=w;
  return xPos;
}

void displayDeviceName(char* deviceName) {
  if (NULL != deviceName) {
    tft.setGFXFont(&FreeSans9pt7b);
    Serial.printf("displayDeviceName: [%s], x=%d, y=%d\n", deviceName, DEVICE_NAME_BLOCK_X, DEVICE_NAME_BLOCK_Y);
    int xPos = getXCentered(String(deviceName), DEVICE_NAME_BLOCK_WIDTH) + DEVICE_NAME_BLOCK_X;
    tft.drawGFXText(xPos , DEVICE_NAME_BLOCK_Y + (DEVICE_NAME_BLOCK_HEIGHT-5), String(deviceName), COLOR_SILVER);
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
