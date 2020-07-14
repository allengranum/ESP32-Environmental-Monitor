#ifndef CONN_MGR_H
#define CONN_MGR_H

#include "EnvMonConfig.h"

#include <WiFi.h>
#include <WifiUDP.h>
#include <NTPClient.h>


#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

class ConnMgr {
    public:
        ConnMgr(EnvMonConfig*);
        void begin();
        void connect();
        WifiInfo* scanForAPs();
        
    private:
        void connectWifi();
        void connectMqtt();

        WiFiClient* wifiClient;
        WiFiUDP *wifiUDP;
        NTPClient *timeClient;
        EnvMonConfig* envMonConfig;
};


#endif