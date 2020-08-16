#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#define DEFAULT_DATA_PUBLISH_INTERVAL_SECONDS 5

typedef enum EnvMonScreen { BLANK_SCREEN, STARTING_SCREEN, CONNECTING_SCREEN, HOME_SCREEN}EnvMonScreen;

class DeviceInfo {
    public:
        DeviceInfo();
        void  begin();
        char* getDeviceName();
        void  setDeviceName(char* name);
        int   getStatusLedState();
        void  setStatusLedState(int);
        int   getStatusLedBrightness();
        void  setStatusLedBrightness(int);
        int   getStatusLedColour();
        void  setStatusLedColour(int);
        float getTemperatureCalibrationValue();
        void  setTemperatureCalibrationValue(float);
        float getTemperature();
        void  setTemperature(float);
        float getHumidityCalibrationValue();
        void  setHumidityCalibrationValue(float);
        float getHumidity();
        void  setHumidity(float);
        char* getSsid();
        void  setSsid(char*);
        EnvMonScreen  getScreen();
        void setScreen(EnvMonScreen);


    private:
            char* deviceName;
            char* ssid;
            char* mqttBroker;
            char* mqttTopicRoot;
            float temperatureCalibrationValue;
            float temperature;
            float humidityCalibrationValue;
            float humidity;
            bool currentMotionDetectedState = false;
            int   statusLedColour;
            int   statusLedBrightness;
            int   displayBacklightBrightness;
            int   displayBacklightState;
            int   orientation;
            EnvMonScreen screen;
};

#endif  // DEVICE_INFO_H