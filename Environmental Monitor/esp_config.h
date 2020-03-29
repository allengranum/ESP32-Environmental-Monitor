#ifndef ESP_CONFIG_H
#define ESP_CONFIG_H

#define CONFIG_FILE_NAME    "/thing.conf"    // ToDo I need the / character???
#define KVP_DELIMITER       ","
#define VALUE_DELIMITER     ";"
#define WIFI_KEY            "WIFI"
#define DEVICE_NAME_KEY     "DEVICE_NAME"
#define MQTT_BROKER_KEY     "MQTT_BROKER"
#define MQTT_TOPIC_ROOT_KEY "MQTT_TOPIC_ROOT"
#define BRIGHTNESS_KEY      "BRIGHTNESS"
#define ORIENTATION_KEY     "ORIENTATION"
#define TEMP_CALIB_KEY      "TEMP_CALIB"
#define HUMIDITY_CALIB_KEY  "HUMIDITY_CALIB"
#define STATUS_LED_BTIGHTNESS_KEY "STATUS_LED_BRIGHTNESS"
#define MAX_WIFI_ENTRIES    20
#define MAX_LINES_IN_CONFIG_FILE 256
#define MAX_LINE_SIZE       256


WifiInfo* getSsids(void);
char* getDeviceName(void);
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

#endif
