#include "UiMgr.h"
#include "EspPins.h"
//#include "bitmaps.h"
//dm #include <../fonts/FreeSans12pt7b.h>
//dm #include <../fonts/FreeSans9pt7b.h>
#include <FreeSans12pt7b.h>
#include <FreeSans12pt7b.h>
#include <FreeSans24pt7b.h>
#include <fonts/FreeSansBold24pt7b.h>
#include <fonts/FreeSans9pt7b.h>

#define TFT_LED 0
#define TFT_RS 19
#define TFT_BRIGHTNESS 150
#define ORIENTATION_0   1
#define ORIENTATION_90  2
#define ORIENTATION_180 3
#define ORIENTATION_270 4

// screen dimensions for the TFT_22_ILI9225
#define LCD_HEIGHT 176
#define LCD_WIDTH  220

// The screen is divided into blocks where the different information will be displayMqttConnectedScreen
// Each block is described with an X/Y coordinate of the upper right corner, and a width and HUMIDITY_BLOCK_HEIGHT

// Temperature Block
#define TEMP_BLOCK_X      5
#define TEMP_BLOCK_Y      0
#define TEMP_BLOCK_HEIGHT 25
#define TEMP_BLOCK_WIDTH  70

// Humidity Block
#define HUMIDITY_BLOCK_X     150
#define HUMIDITY_BLOCK_Y      0
#define HUMIDITY_BLOCK_HEIGHT 25
#define HUMIDITY_BLOCK_WIDTH  70

// Time Block
#define TIME_BLOCK_X       0
#define TIME_BLOCK_Y      50
#define TIME_BLOCK_HEIGHT 40
#define TIME_BLOCK_WIDTH 220

// Motion Icon Block
#define MOTION_ICON_BOX_X 
#define MOTION_ICON_BOX_Y

// Date Block
#define DATE_BLOCK_X        2
#define DATE_BLOCK_Y      100
#define DATE_BLOCK_HEIGHT  25
#define DATE_BLOCK_WIDTH  220

// Device Name Block
#define DEVICE_NAME_BLOCK_X       60
#define DEVICE_NAME_BLOCK_Y      151
#define DEVICE_NAME_BLOCK_HEIGHT  25
#define DEVICE_NAME_BLOCK_WIDTH  100


// NeoPixel Status LED
#define LED_COUNT 1
#define LED_STRIP_INDEX 0
#define DEFAULT_STATUS_LED_BRIGHTNESS  5

// Values used when flashing the status LED
#define FAST_FLASH_uS 500000
#define DATA_READ_INTERVAL_SECONDS 30

// TODO Aren't we now using the DeviceInfo object for this???
extern float g_currentTemp;
extern float g_currentHumidity;
extern int   g_statusLedBrightness;
extern uint32_t g_statusLedColour;
extern int g_statusLedState;

// This was some debug stuff used to see the block size and location
//dm tft.fillRectangle(TEMP_BLOCK_X, TEMP_BLOCK_Y, TEMP_BLOCK_X + TEMP_BLOCK_WIDTH, TEMP_BLOCK_Y + TEMP_BLOCK_HEIGHT, COLOR_ORANGE);
//dm tft.fillRectangle(HUMIDITY_BLOCK_X, HUMIDITY_BLOCK_Y, HUMIDITY_BLOCK_X + HUMIDITY_BLOCK_WIDTH, HUMIDITY_BLOCK_Y + HUMIDITY_BLOCK_HEIGHT, COLOR_SILVER);
//dm tft.fillRectangle(TIME_BLOCK_X, TIME_BLOCK_Y, TIME_BLOCK_X + TIME_BLOCK_WIDTH, TIME_BLOCK_Y+TIME_BLOCK_HEIGHT, COLOR_SKYBLUE);
//dm tft.fillRectangle(DATE_BLOCK_X, DATE_BLOCK_Y, DATE_BLOCK_X + DATE_BLOCK_WIDTH, DATE_BLOCK_Y+DATE_BLOCK_HEIGHT, COLOR_GREEN);
//dm tft.fillRectangle(DEVICE_NAME_BLOCK_X, DEVICE_NAME_BLOCK_Y, DEVICE_NAME_BLOCK_X + DEVICE_NAME_BLOCK_WIDTH, DEVICE_NAME_BLOCK_Y + DEVICE_NAME_BLOCK_HEIGHT, COLOR_TURQUOISE);


void statusLedTimerCallback() {  
  if(LED_ON == myDeviceInfo.getStatusLedState() ) {
    myDeviceInfo.setStatusLedState(LED_OFF);
  } else {
    myDeviceInfo.setStatusLedState(LED_ON);
  }
}

// TODO - Sort this
void _flashStatusLed(int onTime, int offTime) {
  // Flash the status LED during setup
  statusLedTimer = timerBegin(2, 80, true);
  timerAttachInterrupt(statusLedTimer, &statusLedTimerCallback, true);
  timerAlarmWrite(statusLedTimer, FAST_FLASH_uS, true);
//  timerAlarmEnable(statusLedTimer);
}

UiMgr::UiMgr(DeviceInfo* info) {    
    deviceInfo = info;

    neoStatusLed->begin();
    tft->begin();

    currentlyDisplayedScreen = BLANK_SCREEN;
    blackout();

    // Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
    //tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
    // Use software SPI (slower)
    tft = new TFT_22_ILI9225(rst, TFT_RS, cs, mosi, sclk, TFT_LED, TFT_BRIGHTNESS);

    // Status LED
    neoStatusLed = new Adafruit_NeoPixel(LED_COUNT, NEO_STATUS_LED_PIN, NEO_GRB + NEO_KHZ800);

    // Night Light
    nightLight = new Adafruit_NeoPixel(LED_COUNT, NEO_STATUS_LED_PIN, NEO_GRB + NEO_KHZ800);    
}

void UiMgr::begin() {
    activeDisplayOrientation = DISPLAY_ORIENTATION_0;
    setScreen(STARTING_SCREEN);
}

void UiMgr::loop() {
    sync();
}

void UiMgr::blackout() {
    turnOffStatusLed();
    tft->clear();
    activeDisplayBacklightStatus = DISPLAY_BACKLIGHT_OFF;
}

void UiMgr::showStartingScreen() {
    // TODO
}

void UiMgr::setDisplayOrientation(int orientation) {
    // TODO
}

void setBacklightBrightness(int brightness) {
    // TODO
}

void UiMgr::setBackgroundColour(int colour) {
    
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void UiMgr::showDeviceName(char* deviceName) {
    if (NULL != deviceName) {
        tft->setGFXFont(&FreeSans9pt7b);
        Serial.printf("displayDeviceName: [%s], x=%d, y=%d\n", deviceName, DEVICE_NAME_BLOCK_X, DEVICE_NAME_BLOCK_Y);
        int xPos = getXCentered(String(deviceName), DEVICE_NAME_BLOCK_WIDTH) + DEVICE_NAME_BLOCK_X;
        tft->drawGFXText(xPos , DEVICE_NAME_BLOCK_Y + (DEVICE_NAME_BLOCK_HEIGHT-5), String(deviceName), COLOR_SILVER);
    }
}


void UiMgr::turnOnStatusLed() {
    neoStatusLed->setBrightness(g_statusLedBrightness);
    neoStatusLed->setPixelColor(0, g_statusLedColour);
    neoStatusLed->show();
    activeStatusLedState = LED_ON;
}

void UiMgr::turnOffStatusLed() {
    neoStatusLed->setBrightness(0);
    neoStatusLed->setPixelColor(LED_STRIP_INDEX, neoStatusLed->Color(0,   0,   0));
    neoStatusLed->show();
    activeStatusLedState = LED_OFF;
}

void UiMgr::syncStatusLedState() {
    if (activeStatusLedState != g_statusLedState) {
        if(activeStatusLedState == LED_OFF) {
            turnOnStatusLed();
        } else {
            turnOffStatusLed();
        }
    }
   
}

void UiMgr::syncStatusLedColour() {
    if (activeStatusLedColour != g_statusLedColour) {
        neoStatusLed->setPixelColor(LED_STRIP_INDEX, g_statusLedColour);
        neoStatusLed->show();
        activeStatusLedColour = g_statusLedColour;
    }
}

void UiMgr::syncStatusLedBrightness() {
    if (activeStatusLedBrightness != g_statusLedBrightness) {
        neoStatusLed->setBrightness(g_statusLedBrightness);
        neoStatusLed->show();
        activeStatusLedBrightness = g_statusLedBrightness;
    }
}

void UiMgr::syncStatusLed() {
    syncStatusLedState();
    if (activeStatusLedState == LED_ON) {
        syncStatusLedBrightness();
        syncStatusLedColour();
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void UiMgr::syncWifiIcon() {

}

void UiMgr::syncMqttIcon() {

}

void UiMgr::syncTime() {

}

void UiMgr::syncDate() {

}

void UiMgr::syncTemperature() {
    syncTemperature(true);
}

void UiMgr::syncTemperature(boolean force) {

}

void UiMgr::syncHumidity() {
    syncHumidity(true);
}

void UiMgr::syncHumidity(boolean force) {
    char currentHumidityChar[8];
    char activeHumidityChar[8];
  
    tft->setGFXFont(&FreeSans12pt7b);
    if ( force || activeHumidity != currentHumidity) {
        sprintf(activeHumidityChar, "%.1f%%\0", activeHumidity);
        String activeHumidityString = String(activeHumidityChar);
        tft->drawGFXText(HUMIDITY_BLOCK_X, HUMIDITY_BLOCK_Y + HUMIDITY_BLOCK_HEIGHT, activeHumidityString, COLOR_BLACK);
    
        sprintf(currentHumidityChar, "%.1f%%\0", g_currentHumidity);
        String currentHumidityString = String(currentHumidityChar);
        tft->drawGFXText(HUMIDITY_BLOCK_X, HUMIDITY_BLOCK_Y + HUMIDITY_BLOCK_HEIGHT, currentHumidityString, COLOR_BLUE);
    }
}

// void displayTemp(boolean force){
//   char currentTempChar[8];
//   char oldTempChar[8];
  
//   tft.setGFXFont(&FreeSans12pt7b);

//   if (force || oldTemp != currentTemp) {
//     // Delete the old value
//     sprintf(oldTempChar, "%.1fC\0", oldTemp);
//     String oltTempString = String(oldTempChar);
//     tft.drawGFXText(TEMP_BLOCK_X, TEMP_BLOCK_Y + TEMP_BLOCK_HEIGHT, oltTempString, COLOR_BLACK);
  
//     // Draw the new value
//     sprintf(currentTempChar, "%.1fC\0", currentTemp);
//     String currentTempString = String(currentTempChar);
//     tft.drawGFXText(TEMP_BLOCK_X, TEMP_BLOCK_Y + TEMP_BLOCK_HEIGHT, currentTempString, COLOR_BLUE);
//   }
// }

// void displayHumidity(boolean force){
//   char currentHumidityChar[8];
//   char oldHumidityChar[8];
  
//   tft.setGFXFont(&FreeSans12pt7b);
//   if ( force || oldHumidity != currentHumidity) {
//     sprintf(oldHumidityChar, "%.1f%%\0", oldHumidity);
//     String oldHumidityString = String(oldHumidityChar);
//     tft.drawGFXText(HUMIDITY_BLOCK_X, HUMIDITY_BLOCK_Y + HUMIDITY_BLOCK_HEIGHT, oldHumidityString, COLOR_BLACK);
    
//     sprintf(currentHumidityChar, "%.1f%%\0", currentHumidity);
//     String currentHumidityString = String(currentHumidityChar);
//     tft.drawGFXText(HUMIDITY_BLOCK_X, HUMIDITY_BLOCK_Y + HUMIDITY_BLOCK_HEIGHT, currentHumidityString, COLOR_BLUE);
//   }
// }

// void displayTime(boolean force) {
//   if (force || oldTime != currentTime) {
// //    Serial.printf("displayTime: Updating date to: %s\n", t);
// //    Serial.printf("Filling rect: \n");
// //    Serial.printf("X = TIME_BLOCK_X = %d\n", TIME_BLOCK_X);
// //    Serial.printf("Y = TIME_BLOCK_Y = %d\n", TIME_BLOCK_Y);
// //    Serial.printf("X' = LCD_WIDTH - TIME_BLOCK_X = %d - %d = %d\n", LCD_WIDTH, TIME_BLOCK_X, (LCD_WIDTH - TIME_BLOCK_X));
// //    Serial.printf("Y' = TIME_BLOCK_Y + TIME_BLOCK_HEIGHT = %d + %d = %d\n", TIME_BLOCK_Y, TIME_BLOCK_HEIGHT, TIME_BLOCK_Y + TIME_BLOCK_HEIGHT);
//     tft.fillRectangle(TIME_BLOCK_X, TIME_BLOCK_Y, LCD_WIDTH - TIME_BLOCK_X, TIME_BLOCK_Y + TIME_BLOCK_HEIGHT, COLOR_BLACK);
//     tft.setGFXFont(&FreeSansBold24pt7b);
//     int xPos = getXCentered(currentTime, TIME_BLOCK_WIDTH) + TIME_BLOCK_X;
//     tft.drawGFXText(xPos, TIME_BLOCK_Y + TIME_BLOCK_HEIGHT, currentTime, COLOR_RED);
//   }
// }

// void displayDate(boolean force){
//   int val=0;
//   if (force || oldDate != date) {
// //    Serial.printf("displayDate: Updating date to: %s\n", date);
// //    Serial.printf("Filling rect: \n");
// //    Serial.printf("X = DATE_BLOCK_X = %d\n", DATE_BLOCK_X);
// //    Serial.printf("Y = DATE_BLOCK_Y = %d\n", DATE_BLOCK_Y);
// //    Serial.printf("X' = LCD_WIDTH - DATE_BLOCK_X = %d - %d = %d\n", LCD_WIDTH, DATE_BLOCK_X, (LCD_WIDTH - DATE_BLOCK_X));
// //    Serial.printf("Y' = DATE_BLOCK_Y + DATE_BLOCK_HEIGHT = %d + %d = %d\n", DATE_BLOCK_Y, DATE_BLOCK_HEIGHT, DATE_BLOCK_Y + DATE_BLOCK_HEIGHT);
//     tft.fillRectangle(DATE_BLOCK_X, DATE_BLOCK_Y, LCD_WIDTH - DATE_BLOCK_X, DATE_BLOCK_Y + DATE_BLOCK_HEIGHT, COLOR_BLACK);
//     tft.setGFXFont(&FreeSans12pt7b);
//     int xPos = getXCentered(date, DATE_BLOCK_WIDTH) + DATE_BLOCK_X;
//     tft.drawGFXText(xPos, DATE_BLOCK_Y + DATE_BLOCK_HEIGHT - 5, date, COLOR_YELLOW);
//   }
// }

// void displayWifi(boolean force){
// }

// void displayAll(boolean force) {
//   displayWifi(force);
//   displayDate(force);
//   displayTime(force);
//   displayTemp(force);
//   displayHumidity(force);  
// }


void displayDateTime(boolean force){
  displayTime(force);
  displayDate(force);
}


void UiMgr::syncData() {
    syncTemperature();
    syncHumidity();
}

void UiMgr::syncLcd() {
    syncWifiIcon();
    syncMqttIcon();
    syncTime();
    syncDate();
    syncData();
    syncLcdBrightness();
    syncLcdBacklight();
}

void syncMotion() {
    syncMotion(false);
}

void syncMotion( boolean force) {
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
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void UiMgr::syncBuzzer() {

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void UiMgr::syncNightLight() {

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void UiMgr::displayWifiConnectingScreen() {
    tft->clear();
    tft->setFont(Terminal6x8);
    String myString = "Connecting to: " + String(deviceInfo->getSsid());
    tft->drawText(TIME_BLOCK_X, TIME_BLOCK_Y - 20, myString, COLOR_YELLOW);
}

void UiMgr::displayWifiConnected(char* ssid) {
  tft->clear();
//    display.setTextSize(1);
//    display.setCursor(TIME_BLOCK_X,TIME_BLOCK_Y - 30);
//    display.setTextColor(GREEN);
//    display.println("Connected to: ");
//    display.setTextColor(YELLOW);
//    display.println(ssid);
//    display.println("");
//    display.println(WiFi.localIP());
}

void UiMgr::displayHomeScreen() {

}

void UiMgr::displayConfigScreen() {

}

void UiMgr::setScreen(EnvMonScreen screen) {
    switch (screen) {
        case BLANK_SCREEN:
            blackout();
            break;

        case STARTING_SCREEN:
            displayStartingScreen();
            break;

        case CONNECTING_SCREEN:
            displayWifiConnectingScreen();
            break;

        case HOME_SCREEN:
            break;
    }
    currentlyDisplayedScreen = screen;
}

void UiMgr::sync()
{
    switch (currentlyDisplayedScreen) {
        case BLANK_SCREEN:
            break;

        case STARTING_SCREEN:
            break;

        case CONNECTING_SCREEN:
            if (deviceInfo->wifiConnected())
                if (deviceInfo->mqttConnected())
                    displayMqttConnectedScreen();
            break;

        case HOME_SCREEN:
            break;

        default:
            break;
    }

    syncStatusLed();
    syncLcd();
    syncBuzzer();
    syncNightLight();

    // int value = (int)atoi(payloadStr);
// //    Serial.printf("Turning backlight %s", value = 0 ? "off" "on");
//     if (0 == value) {
//       Serial.println("backlight setting to false");
//       tft.setBacklight(false);
//     } else {
//       Serial.println("backlight setting to true");
//       tft.setBacklight(false);

}

//------------------------------------------------------------------------------
int UiMgr::getXCentered(String string, int width) {
  int16_t x1, y1, xPos;
  int16_t w, h;

//  Serial.print("getXCentered: string=[");
//  Serial.print(string);
//  Serial.println("]");
//  Serial.print("String length = ");
//  Serial.println(string.length());
  // Determine coordinates to center the time
  tft->getGFXTextExtent(string, x1, y1, &w, &h);
//  Serial.print("width of string = ");
//  Serial.println(w);
  xPos = (width-w)/2;
//  Serial.print("calculated xPos = ");
//  Serial.println(xPos);
//  Serial.println("");
//  Serial.println("");  
  return xPos;
}

int UiMgr::getXRight(float val) {
  int16_t x1, y1, xPos;
  int16_t w, h;
  char buff[10];

  dtostrf(val, 3, 0, buff);  //4 is mininum width, 6 is precision
  // Determine coordinates to center the time
  tft->getGFXTextExtent(buff, x1, y1, &w, &h);
  xPos = (LCD_HEIGHT-w);
  tft->getGFXTextExtent("%", x1, y1, &w, &h);
  xPos-=w;
  return xPos;
}