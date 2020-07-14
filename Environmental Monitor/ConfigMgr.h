#ifndef CONFIG_MGR_H
#define CONFIG_MGR_H

#include "SPIFFS.h"

#define MAX_CONFIG_FILE_NAME_SIZE 256
#define MAX_LINES_IN_CONFIG_FILE  256
#define MAX_LINE_SIZE             256
#define MAX_WIFI_ENTRIES          20

#define KVP_DELIMITER             ","   // key;value
#define VALUE_DELIMITER           ";"   // value_element_1:value_element_2:value_element_3

#define WIFI_KEY                  "WIFI"
#define DEVICE_NAME_KEY           "DEVICE_NAME"

typedef struct wifiInfo_t{
  int numberOfSSIDsFound;
  char** ssid;
}WifiInfo;

class ConfigMgr {
    public:
        ConfigMgr(char* configFileName);

        void    begin();

        char*   getDeviceName(void);
        void    setDeviceName(char*);

        void    setWifiInfo(char*, char*);

        char*   getCustomValue(char*);
        int     getCustomValueInt(char*);
        float   getCustomValueFloat(char*);


        boolean keyExists(char*);
        boolean keyValueExists(char*, char*);

        char*   getKey(char*);
        char*   getValue(char*);
        boolean setKeyValue(char*, char*, boolean);

        void    deleteKey(char*);
        void    deleteKeyValue(char*, char*);
        
        WifiInfo* getSsids();

        char* getWifiPasswordFromValue(char*);
        void    dumpConfig();
        void    deleteConfigData(void);

    private:
        boolean initialize(void);
        char*   getNextLine(File );
        boolean lineMatchesKey(char*, char*);

        char configFileName[256];
        boolean initialized = false;
};

#endif