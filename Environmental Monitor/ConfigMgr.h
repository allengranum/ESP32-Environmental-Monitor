#ifndef CONFIG_MGR_H
#define CONFIG_MGR_H

#define MAX_CONFIG_FILE_NAME_SIZE 256;

class ConfigMgr {
    public:
        ConfigMgr(char* configFileName);
        void begin();
        int getDisplayBackligtBrightness();
        void setTemperatureCalibration(float);
        void setHumidityCalibration(float);

    private:
        char configFileName[256];
        char* getCustomValue(char* key);
        int getCustomValueInt(char* key);
        float getCustomValueFloat(char* key);

        float humidityCalibrationValue;
        float temperatureCalibrationValue;
};

#endif