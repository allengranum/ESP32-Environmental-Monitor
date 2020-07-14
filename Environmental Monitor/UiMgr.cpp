#include "UiMgr.h"
#include "EspPins.h"
#include <../fonts/FreeSans12pt7b.h>
#include <../fonts/FreeSans9pt7b.h>

#define TFT_LED 0
#define TFT_RS 19
#define TFT_BRIGHTNESS 150
#define ORIENTATION_0   1
#define ORIENTATION_90  2
#define ORIENTATION_180 3
#define ORIENTATION_270 4


#define LCD_HEIGHT 176
#define LCD_WIDTH  220

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

// NeoPixel Status LED
#define LED_COUNT 1
#define LED_STRIP_INDEX 0
#define DEFAULT_STATUS_LED_BRIGHTNESS  5

extern float g_currentTemp;
extern float g_currentHumidity;
extern int   g_statusLedBrightness;
extern uint32_t g_statusLedColour;
extern int g_statusLedState;

UiMgr::UiMgr(DeviceInfo* info) {    
    deviceInfo = info;
    screen = BLANK_SCREEN;

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
    neoStatusLed->begin();
    turnOffStatusLed();

    tft->begin();
    tft->clear();    
    activeDisplayOrientation = DISPLAY_ORIENTATION_0;
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

void syncDate() {

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
   // if ( force || activeHumidity != currentHumidity) {
        sprintf(activeHumidityChar, "%.1f%%\0", activeHumidity);
        String activeHumidityString = String(activeHumidityChar);
        tft->drawGFXText(HUMIDITY_BLOCK_X, HUMIDITY_BLOCK_Y + HUMIDITY_BLOCK_HEIGHT, activeHumidityString, COLOR_BLACK);
    
        sprintf(currentHumidityChar, "%.1f%%\0", g_currentHumidity);
        String currentHumidityString = String(currentHumidityChar);
        tft->drawGFXText(HUMIDITY_BLOCK_X, HUMIDITY_BLOCK_Y + HUMIDITY_BLOCK_HEIGHT, currentHumidityString, COLOR_BLUE);
   // }
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void UiMgr::syncBuzzer() {

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void UiMgr::syncNightLight() {

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void UiMgr::displayWifiConnecting() {
    tft->clear();
    tft->setFont(Terminal6x8);
    String myString = "Connecting to: " + String(deviceInfo->getSsid());
    tft->drawText(TIME_BLOCK_X, TIME_BLOCK_Y - 20, myString, COLOR_YELLOW);
}

void UiMgr::setScreen(EnvMonScreen screen) {
    screen = screen;
}

void UiMgr::sync()
{
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