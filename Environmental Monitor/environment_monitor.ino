#include "environment_monitor.h"
#include "DeviceInfo.h"
#include "esp_onboarding.h"
#include "UiMgr.h"
#include "EnvMonConfig.h"
#include "GpioMgr.h"
#include "ConnMgr.h"
#include "TelemetryMgr.h"
#include "EspPins.h"

#include <SPI.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

//
// Instantiate the components of the software
//
DeviceInfo   myDeviceInfo;  // This stores all of the information about the device and it's current state
UiMgr        uiMgr;         // Responsible for updating the display screen
GpioMgr      gpioMgr;       // Talks to th esensors and actuators
ConnMgr      connMgr;       // Responsible for WiFi and MQTT server connections
EnvMonConfig envMonConfig;  // Responsible for storing and retrieving device config stuff from flash
TelemetryMgr telemetryMgr;  // responsible for sending sensor data when appropriate

// USE THE TIMER SERVICE INSTEAD
//dm hw_timer_t * dataCollectionTimer = NULL;
//dm hw_timer_t * wifiConnectTimer = NULL;
//dm hw_timer_t * statusLedTimer = NULL;

//int g_orientation = DISPLAY_ORIENTATION_0;
//uint32_t g_statusLedColour;
//int g_statusLedState;
//int g_statusLedBrightness;
//int g_displayBacklightBrightness;
//int g_displayBacklightState;

void initializeDevice() {
  myDeviceInfo.setStatusLedBrightness( envMonConfig.getStatusLedBrightness() );
  myDeviceInfo.setTemperatureCalibrationValue( envMonConfig.getTemperatureCalibrationValue() );
  myDeviceInfo.setHumidityCalibrationValue( envMonConfig.getHumidityCalibrationValue() );
  myDeviceInfo.setStatusLedColour(NEOCOLOUR_RED);

  uiMgr.setDisplayOrientation(DISPLAY_ORIENTATION_0);
  uiMgr.setBackgroundColour(COLOR_BLACK);
  uiMgr.showStartingScreen();
  uiMgr.setBacklightBrightness(envMonConfig.getDisplayBackligtBrightness());
  uiMgr.showDeviceName(envMonConfig.getDeviceName());

  pinMode(DHT_SENSOR_PIN, INPUT);
  pinMode(CONFIG_BUTTON, INPUT_PULLUP);
  pinMode(DELETE_CONFIG_BUTTON, INPUT_PULLUP);
  // pinMode(STATUS_LED, OUTPUT);
  pinMode(MOTION_SENSOR_PIN, INPUT);
  delay(100); // There needs to be some delay here before immediately performing
              // a digitalRead on a pin or else the results are non deterministic

  int buttonState = gpioMgr.buttonInit();
  if (buttonState) {
    performOnboarding();
  }
}

void setup() {

  Serial.begin(9600);

  myDeviceInfo.begin();
  envMonConfig.begin();
  uiMgr.begin();
  gpioMgr.begin();
  connMgr.begin();
  telemetryMgr.begin();

  initializeDevice();

  uiMgr.setStatusLedColour(GREEN);
  uiMgr.flashStatusLed(true);

  delay(1000); // this is magical. if you take it out, things go bad
  envMonConfig.dumpConfig();

  uiMgr.setScreen(CONNECTING_SCREEN); //displayWifiConnecting();
  connMgr.connect();

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

  uiMgr.setStatusLedColour(GREEN);
  uiMgr.flashStatusLed(false);
  uiMgr.setScreen(HOME_SCREEN);
}

void loop() {
  
  // Now just call all of the sub-component loop functions
  // and let them all take care of their respective areas
  // of responsability.

  gpioMgr.loop();        // Read the sensor data
  uiMgr.loop();          // Update the display with the sensor data
  connMgr.loop();        // check or WiFi and MQTT connections
  telemetryMgr.loop();   // Send the sensor data if connected
  envMonConfig.loop();      // Update any values in flash if required


  // if (WiFi.status() == WL_CONNECTED) {
  //   if (g_mqttConnected) {

  //     mqttClient.loop();

  //     readData();
      
  //     uiMgr.sync();

  //     // configMgr.sync();

  //     if (timeToSendData) {
  //       sendData();
  //       timeToSendData = false;
  //     }

  //     boolean motionStateChanged = readMotionData();
  //     if (motionStateChanged) {
  //       sendMotionData();
  //     }

  //   } else {
  //     mqttReconnectCount++;
  //     g_mqttConnected = mqttConnectLocal(getMqttBroker(), mqttCallback, g_deviceName);
  //   }
  // } else {
  //   Serial.println("WiFi not connected. Re-trying connection");
  //   wifiReconnectCount++;
  //   connectToWifi();
  // }
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

void displayWifiConnected(char* ssid) {
//    tft.clear();
//    display.setTextSize(1);
//    display.setCursor(TIME_BLOCK_X,TIME_BLOCK_Y - 30);
//    display.setTextColor(GREEN);
//    display.println("Connected to: ");
//    display.setTextColor(YELLOW);
//    display.println(ssid);
//    display.println("");
//    display.println(WiFi.localIP());
}



//-------------------------------------------------------------------------------








char* string2char(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
}

// void criticalCodeStart() {
// //  Serial.println("Crit section start..............");
//   noInterrupts();
//   timerEnd(dataCollectionTimer);
// }

// void criticalCodeEnd() {
//   interrupts();
//   timerAlarmEnable(dataCollectionTimer);
// //  Serial.println("Crit section enc ^^^^^^^^^^^^^^^");
// }



