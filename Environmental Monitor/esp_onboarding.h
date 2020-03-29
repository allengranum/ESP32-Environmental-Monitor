#ifndef ESP_ONBOARDING_H
#define ESP_ONBOARDING_H

typedef struct wifiInfo_t{
  int numberOfSSIDsFound;
  char** ssid;
}WifiInfo;

void performOnboarding(void);
WifiInfo* scanForAPs(void);

#endif
