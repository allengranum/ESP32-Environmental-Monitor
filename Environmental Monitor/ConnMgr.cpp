#include "ConnMgr.h"

#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

ConnMgr::ConnMgr(EnvMonConfig* config) {    
    envMonConfig = config;
    wifiClient = new WiFiClient();
    wifiUDP = new WiFiUDP();
    timeClient = new NTPClient(*wifiUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
}

void ConnMgr::begin() {
    
}

void ConnMgr::connect() {
    connectWifi();
    connectMqtt();
}

void ConnMgr::connectMqtt() {
    
}

void ConnMgr::connectWifi() {
//  timerEnd(wifiConnectTimer);
  
  // Get the list of known wifi networks from the config file
  WifiInfo* knownNetworks = envMonConfig->getSsids();
  
  if (knownNetworks->numberOfSSIDsFound > 0) {
    Serial.printf("connectToWifi: %d known networks found in config file\n", knownNetworks->numberOfSSIDsFound);
    delay(500);
    for (int i=0;i<knownNetworks->numberOfSSIDsFound;i++) {
      Serial.printf("    %d: [%s]\n", i, knownNetworks->ssid[i]);
    }

    // Get a list of available wifi networks
    WifiInfo* availableNetworks = scanForAPs();
        
    char* ssid = NULL;
    char* password = NULL;
    int index=0;
    
    while( index < availableNetworks->numberOfSSIDsFound && ssid == NULL ) {
      char* currentSsid = availableNetworks->ssid[index];
      Serial.printf("Checking if I know about ssid [%s]\n", currentSsid);
      // See if it exists in the known list 
      for (int i=0;i<knownNetworks->numberOfSSIDsFound;i++) {
        if (0 == strcmp(currentSsid, knownNetworks->ssid[i])) {
          ssid = knownNetworks->ssid[i];
        }
      }

      // if yes, get the associated passsword and attempt to connect
      if (NULL != ssid) {
        Serial.printf("connectToWifi: Attempting to connect to SSID: [%s]\n", ssid);
        password = envMonConfig->getWifiPassword(ssid);
        Serial.printf("connectToWifi: Using password: [%s]\n", password);

        displayWifiConnecting(ssid);
                
        WiFi.begin(ssid, password);
        
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }

        displayWifiConnected(ssid);
        
        Serial.printf("WiFi connected\n");
        Serial.printf("IP address: \n");
        Serial.println(WiFi.localIP());
      } else {
          Serial.printf("I don't know any of the available WiFi networks.\n");
      }
      index++;
      // if no, get the next available network from the top of the available list and try again    
    }
    
    // Free up memory
    free(availableNetworks->ssid);
    free(availableNetworks);
    free(knownNetworks->ssid);

    if (WiFi.status() != WL_CONNECTED) {
      performOnboarding();
    }
  } else {
    Serial.println("connectToWifi: need to perform onboarding");
    delay(500);
    performOnboarding();
  }   
}

WifiInfo* ConnMgr::scanForAPs() {
  WifiInfo* result = (WifiInfo*)malloc(sizeof(WifiInfo));
  wifi_mode_t savedMode = WiFi.getMode();

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.printf("scanForAPs: Detected %d network access points\n", n);
    
    // Sort the indexes of the SSIDs discovered based on signal strength
    int* sortedIndexes = (int*)malloc(n * sizeof(int));
    for (int i=0;i<n;i++) {
      sortedIndexes[i] = 0;
    }
    
    for (int i=0;i<n;i++) {
      for (int j=0;j<n;j++) {
        if (WiFi.RSSI(j) > WiFi.RSSI(i)) {
          sortedIndexes[i] = j;
        }
      }
    }

    // Remove duplicate SSID entries
    char** resultSsidList = (char**)malloc(n * sizeof(char*));
    
    for (int i=0;i<n;i++) {
      boolean alreadyInList = false;
      int size = WiFi.SSID(sortedIndexes[i]).length();
      char * tempBuf = (char*) malloc((size + 1) * sizeof(char));
      WiFi.SSID(sortedIndexes[i]).toCharArray(tempBuf, size + 1);
          
      // Check to see if this AP has the same name as one already in our list
      if (numberOfUniqueSSIDsDiscovered > 0) {
        for (int i=0;i<numberOfUniqueSSIDsDiscovered;i++) {
          if (0 == strcmp(discoveredSSIDs[i], tempBuf)) {
            alreadyInList = true;
          }
        }
      }
      
      // If it isn't in our list, add it
      if (!alreadyInList) {
        resultSsidList[numberOfUniqueSSIDsDiscovered] = tempBuf;
        strcpy(discoveredSSIDs[numberOfUniqueSSIDsDiscovered], tempBuf);
        numberOfUniqueSSIDsDiscovered++;
      } else {
        free(tempBuf);
      }
    }

    result->ssid = resultSsidList;
    result->numberOfSSIDsFound = numberOfUniqueSSIDsDiscovered;
    
    Serial.printf("Found %d unique SSID", result->numberOfSSIDsFound);
    if (result->numberOfSSIDsFound >1 ) {
      Serial.printf("s");
    }
    Serial.printf("\n");
    
    WiFi.mode(savedMode);
  }
  return result;
}
