#include "EnvMonMqtt.h"
#include "UiMgr.h"

#include <WiFi.h>
#include <PubSubClient.h>

void EnvMonMqtt::mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  char* payloadStr = (char*)malloc((length+1) * sizeof(char));
  memcpy(payloadStr, payload, length);
  payloadStr[length] = '\0';

  char* command;
  char* token;
  char* lastToken;

  token = strtok((char*)topic, "/");
  
  while (NULL != token) {
    lastToken = token;
    token = strtok(NULL, "/");
  }
  command = lastToken;

  // Brightness -------------------------
  if (0 == strcmp(command, SUB_TOPIC_BRIGHTNESS)) {
    deviceStateInfo->displayBacklightBrightness = (int)atoi(payloadStr);    
  } else
  // Backlight -------------------------
  if (0 == strcmp(command, SUB_TOPIC_BACKLIGHT)) {
    deviceStateInfo->displayBacklightState = (int)atoi(payloadStr);
  } else
  // flip -------------------------
  if (0 == strcmp(command, SUB_TOPIC_FLIP)) {
    Serial.println("flipping the display");
    if (deviceStateInfo->orientation == DISPLAY_ORIENTATION_0) {
      deviceStateInfo->orientation = DISPLAY_ORIENTATION_180;
    } else {
      deviceStateInfo->orientation = DISPLAY_ORIENTATION_0;
    }
  } else
  // temp calibration -------------------------
  if (0 == strcmp(command, SUB_TOPIC_TEMP_CALIB)) {
    float value = (float)atof(payloadStr);
    envMonConfig.setTemperatureCalibrationValue(payloadStr);
    deviceStateInfo->temperatureCalibrationValue = value;
  } else
  // humidity calibration -------------------------
  if (0 == strcmp(command, SUB_TOPIC_HUMIDITY_CALIB)) {
    Serial.println("Adjusting humidity calibration");
    float value = (float)atof(payloadStr);
    envMonConfig.setHumidityCalibrationValue(payloadStr);
    deviceStateInfo->humidityCalibrationValue = value;
  } else
  // Neo Status LED brightness -------------------------
  if (0 == strcmp(command, SUB_TOPIC_LED_BRIGHTNESS)) {
//    Serial.println("Adjusting led brightness");
    int value = (int)atoi(payloadStr);
    if (value >= 0 && value <= 255) {
      envMonConfig.setStatusLedBrightness(value);
    } else {
      Serial.printf("Invalid value for orientation: %d\n", value);
    }
  } else 
  // Neo Status LED colour -------------------------
  if (0 == strcmp(command, SUB_TOPIC_LED_COLOUR)) {
    Serial.println("Setting led colour");
//    neoStatusLed.setPixelColor(1, g_statusLedColour);
//    neoStatusLed.show();
  } else {
    Serial.printf("#### unhandled command [%s]####\n", command);
  }
    
}


EnvMonMqtt::EnvMonMqtt(EnvMonConfig config, DeviceStateInfo *devStateInfoPtr, WiFiClient *wifiClient) {    
    envMonConfig = config;
    deviceStateInfo = devStateInfoPtr;
    mqttConnected = false;
    espClient = wifiClient;
    mqttClient = new PubSubClient(*espClient);
}

void EnvMonMqtt::begin() {
    
}
void EnvMonMqtt::init() {
  myDeviceInfo.deviceName = envMonConfig.getDeviceName();
  myDeviceInfo.mqttTopicRoot = envMonConfig.getMqttTopicRoot();
  
  // Build the root MQTT topic for the device that  it will
  // subscribe to for receiving device commands
  sprintf(g_mqttTopicDevice, "/%s/cmd/+", deviceStateInfo->deviceName);
  Serial.printf("g_mqttTopicDevice = [%s]\n", g_mqttTopicDevice);
  
  sprintf(g_topic_brightness, "%s/%s/%s", MQTT_TOPIC_ROOT, deviceStateInfo->deviceName, SUB_TOPIC_BRIGHTNESS);
  Serial.printf("g_topic_brightness = [%s]\n", g_topic_brightness);

  sprintf(g_topic_orientation, "%s/%s/%s", MQTT_TOPIC_ROOT, deviceStateInfo->deviceName, SUB_TOPIC_ORIENTATION);
  Serial.printf("g_topic_orientation = [%s]\n", g_topic_orientation);

  sprintf(g_topic_temp_calib, "%s/%s/%s", MQTT_TOPIC_ROOT, deviceStateInfo->deviceName, SUB_TOPIC_TEMP_CALIB);
  Serial.printf("g_topic_temp_calib = [%s]\n", g_topic_temp_calib);

  sprintf(g_topic_humidity_calib, "%s/%s/%s", MQTT_TOPIC_ROOT, deviceStateInfo->deviceName, SUB_TOPIC_HUMIDITY_CALIB);
  Serial.printf("g_topic_humidity_calib = [%s]\n", g_topic_humidity_calib);

  mqttClient->setServer(envMonConfig.getMqttBroker(), 1883);
  mqttClient->setCallback(mqttCallback);
  mqttConnected = mqttConnectLocal(envMonConfig.getMqttBroker(), mqttCallback, deviceStateInfo->deviceName);
  if (mqttConnected) {
    char topic[256];
    char value[2];

    sprintf(topic, "%s/%s/%s", MQTT_TOPIC_ROOT, deviceStateInfo->deviceName, SUB_TOPIC_CONNECTED);
    sprintf(value, "1");
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient->publish(&topic[0], value, true);
    sprintf(topic, "%s/%s/%s", MQTT_TOPIC_ROOT, deviceStateInfo->deviceName, SUB_TOPIC_TEMP_CALIB);
    sprintf(value, "", deviceStateInfo->temperatureCalibrationValue);
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient->publish(&topic[0], value, true);

    // g_statusLedBrightness = getStatusLedBrightness();
    // g_temperatureCalibrationValue = getTemperatureCalibrationValue();
    // g_humidityCalibrationValue = getHumidityCalibrationValue();
  }
}