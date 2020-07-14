#ifndef UI_MGR_H
#define UI_MGR_H

#include "DeviceInfo.h"
#include <TFT_22_ILI9225.h>
#include <Adafruit_NeoPixel.h>

#define LED_OFF 0
#define LED_ON  1

#define NEOCOLOUR_RED   0xff0000
#define NEOCOLOUR_GREEN 0x00ff00

#define DISPLAY_BACKLIGHT_OFF 0
#define DISPLAY_BACKLIGHT_ON  1

#define DISPLAY_ORIENTATION_0   3
#define DISPLAY_ORIENTATION_180 1

class UiMgr {
    public:
        UiMgr(DeviceInfo* info);
        void begin();
        void sync();
        void showStartingScreen();
        void setDisplayOrientation(int);
        void setBackgroundColour(int);
        void setBacklightBrightness(int);
        void showDeviceName(char*);
        void displayWifiConnecting();
        void setScreen(EnvMonScreen);
        
    private:
        void syncStatusLedBrightness();
        void syncStatusLedColour();
        void syncTemperature(void);
        void syncTemperature(boolean);
        void syncHumidity();
        void syncHumidity(boolean);
        void syncTime();
        void syncDate();
        void syncData();
        void syncLcdBrightness();
        void syncLcdBacklight();
        void syncLcd();
        void syncStatusLed();
        void syncStatusLedState();
        void turnOffStatusLed();
        void turnOnStatusLed();
        void syncNightLight();
        void syncBuzzer();
        void syncWifiIcon();
        void syncMqttIcon();
        void showDeviceName(char*);
        int  getXRight(float val);
        int  getXCentered(String string, int width);
        void blackout();

        TFT_22_ILI9225 *tft;
        Adafruit_NeoPixel *neoStatusLed;
        Adafruit_NeoPixel *nightLight;

        DeviceInfo* deviceInfo;
        EnvMonScreen screen;
        float activeTemp;
        float activeHumidity;
        char* activeTime;
        char* activeDate;
        char* activeDeviceName;
        int   activeStatusLedBrightness;
        int   activeStatusLedColour;
        int   activeStatusLedState; // 0 = OFF, 1 = ON
        int   activeDisplayBacklightStatus; 
        int   activeDisplayOrientation;
};

#endif