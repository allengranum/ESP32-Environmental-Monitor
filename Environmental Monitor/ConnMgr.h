#ifndef CONN_MGR_H
#define CONN_MGR_H

#include "EnvMonConfig.h"

#include <WiFi.h>
#include <WifiUDP.h>
#include <NTPClient.h>

// NTP & time stuff
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

typedef enum CON_MGR_STATUS { 
    CON_MGR_SUCCESS=0, 
    NO_WIFI_INFO,
    NO_WIFI_MATCH,
    NO_MQTT_INFO,
    MQTT_FAIL
} CON_MGR_STATUS;


class ConnMgr {
    public:
        ConnMgr(EnvMonConfig*);
        void begin();
        void loop();
        CON_MGR_STATUS connect();
        WifiInfo* scanForAPs();
        void readDateTime();
        
    private:
        CON_MGR_STATUS connectWifi();
        CON_MGR_STATUS connectMqtt();

        bool connectingToWifi;
        bool wifiConnected;
        WiFiClient*   wifiClient;
        WiFiUDP*      wifiUDP;
        NTPClient*    timeClient;
        EnvMonConfig* envMonConfig;
        String        date;
        String        time;
};


#endif