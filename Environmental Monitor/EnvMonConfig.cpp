#include "EnvMonConfig.h"
#include "environment_monitor.h"

EnvMonConfig::EnvMonConfig() {    
    configMgr = new ConfigMgr(ENV_MON_CONFIG_FILE_NAME);
}

void EnvMonConfig::begin() {
    configMgr->begin();
}

char* EnvMonConfig::getDeviceName() {
    return configMgr->getDeviceName();
}

void EnvMonConfig::setDeviceName(char* name) {
    return configMgr->setDeviceName(name);
}

float EnvMonConfig::getTemperatureCalibrationValue() {
    return configMgr->getCustomValueFloat(TEMP_CALIB_KEY);
}

void EnvMonConfig::setTemperatureCalibrationValue(char* value) {
    configMgr->setKeyValue( TEMP_CALIB_KEY, value, true);
}

float EnvMonConfig::getHumidityCalibrationValue() {
    return configMgr->getCustomValueFloat(HUMIDITY_CALIB_KEY);;
}

void EnvMonConfig::setHumidityCalibrationValue(char* value) {
    configMgr->setKeyValue( HUMIDITY_CALIB_KEY, value, true);
}

void EnvMonConfig::setStatusLedBrightness(int value) {
  char valueStr[4];
  sprintf(valueStr, "%d", value);
  configMgr->setKeyValue( STATUS_LED_BTIGHTNESS_KEY, valueStr, true);
}

int EnvMonConfig::getStatusLedBrightness() {
    return configMgr->getCustomValueInt(STATUS_LED_BTIGHTNESS_KEY);
}

int EnvMonConfig::getDataPublishInterval() {
    int value = configMgr->getCustomValueInt(DATA_PUBLISH_INTERVAL_KEY);

    if (value == 0) {
      value = DEFAULT_DATA_PUBLISH_INTERVAL_SECONDS;
    }
    Serial.printf("getDataPublishInterval = [%d]\n", value);
    return value;
}

WifiInfo* EnvMonConfig::getSsids() {
    return configMgr->getSsids();
}

char* EnvMonConfig::getMqttBroker() {
  return configMgr->getCustomValue(MQTT_BROKER_KEY);
}

char* EnvMonConfig::getMqttTopicRoot() {
  return configMgr->getCustomValue(MQTT_TOPIC_ROOT_KEY);
}

void EnvMonConfig::deleteConfigData() {
    configMgr->deleteConfigData();
}

void EnvMonConfig::dumpConfig() {
    configMgr->dumpConfig();
}


char* EnvMonConfig::getWifiPassword() {
  return configMgr->getWifiPassword(MQTT_TOPIC_ROOT_KEY);
}