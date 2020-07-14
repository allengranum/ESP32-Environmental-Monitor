#ifndef ENV_MON_CONFIG_H
#define ENV_MON_CONFIG_H

#include "ConfigMgr.h"

#define ENV_MON_CONFIG_FILE_NAME "/thing.conf"    // ToDo I need the / character???

#define MQTT_BROKER_KEY           "MQTT_BROKER"
#define MQTT_TOPIC_ROOT_KEY       "MQTT_TOPIC_ROOT"
#define BRIGHTNESS_KEY            "BRIGHTNESS"
#define ORIENTATION_KEY           "ORIENTATION"
#define TEMP_CALIB_KEY            "TEMP_CALIB"
#define HUMIDITY_CALIB_KEY        "HUMIDITY_CALIB"
#define STATUS_LED_BTIGHTNESS_KEY "STATUS_LED_BRIGHTNESS"
#define DATA_PUBLISH_INTERVAL_KEY "DATA_PUBLISH_INTERVAL_SECONDS"

class EnvMonConfig {
    public:
        EnvMonConfig();
        void begin();
        char* getDeviceName();
        void  setDeviceName(char*);
        char* getWifiPassword();
        void  setWifiPassword(char*);
        float getTemperatureCalibrationValue();
        void  setTemperatureCalibrationValue(char* value);
        float getHumidityCalibrationValue();
        void  setHumidityCalibrationValue(char* value);
        WifiInfo* getSsids();
        char* getMqttBroker();
        char* getMqttTopicRoot();
        int   getDataPublishInterval();
        int   getStatusLedBrightness();
        void  setStatusLedBrightness(int value);
        int   getDisplayBackligtBrightness();
        void  setDisplayBackligtBrightness(int value);
        void  deleteConfigData();
        void  dumpConfig();

/*
WifiInfo* getSsids(void);
void setDeviceName(char*);
void saveWifiInfo(char*, char*);
void dumpConfig(void);
char* getWifiPassword(char*);
void deleteConfigData(void);
char* getCustomValue(char*);
float getCustomValueFloat(char*);
int getCustomValueInt(char*);
char* getOrientation(void);
char* getBrightness(void);
char* getMqttTopicRoot(void);
boolean saveKeyValue(char* , char* , boolean);
*/




    private:
        ConfigMgr *configMgr;
        float g_temperatureCalibrationValue;
};


#endif // ENV_MON_CONFIG_H