#include "EnvMonMqtt.h"
#include "UiMgr.h"

#include <WiFi.h>
#include <PubSubClient.h>

boolean g_mqttConnected = false;
int mqttReconnectCount = 0;

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




boolean EnvMonMqtt::mqttConnectLocal(char* broker, void (*callback)(char*, uint8_t*, unsigned int), const char* deviceName) {
  boolean retVal = false;
  
  if (broker) {
    Serial.printf("mqttConnect: Connecting to mqttBroker = [%s]\n", broker);  
//  delay(2000);
    mqttClient.setServer(broker, 1883);
    mqttClient.setCallback(mqttCallback);    
//    Serial.printf("Attempting to connect to MQTT Broker [%s]\n", deviceName);
//    delay(2000);
    if (mqttClient.connect(deviceName)) {
      Serial.printf("Connected to MQTT broker [%s]\n", broker);
      mqttClient.subscribe(g_mqttTopicDevice);
//      boolean result = mqttClient.subscribe("inTopic");
//      if (!result) {
//        Serial.println("ERROR: Failed to subscribe to topic inTopic");
//      }
      Serial.printf("Subscribed to topic: [%s]\n", g_mqttTopicDevice);
//      Serial.printf("Subscribed to topic: [%s]\n","inTopic");
    } else {
      Serial.printf("mqttConnect: ERROR: Failed to connect to MQTT broker [%s]\n", broker);
    }

    if (mqttClient.connected()) {
      retVal = true;
      Serial.println("MQTT connection verified");
    } else {
      Serial.println("Unable to verify MQTT connectio");
    }
  } else {
    Serial.println("mqttConnect: ERROR: Unable to connect. broker = NULL");
  }
  return retVal;
}

void EnvMonMqtt::reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("outTopic", "hello world");
      // ... and resubscribe
      mqttClient.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// void mqttCallback(char* topic, byte* payload, unsigned int length) {
//   Serial.print("Message arrived [");
//   Serial.print(topic);
//   Serial.print("] ");
//   for (int i = 0; i < length; i++) {
//     Serial.print((char)payload[i]);
//   }
//   Serial.println();
//   char* payloadStr = (char*)malloc((length+1) * sizeof(char));
//   memcpy(payloadStr, payload, length);
//   payloadStr[length] = '\0';

//   char* command;
//   char* token;
//   char* lastToken;

//   token = strtok((char*)topic, "/");
  
//   while (NULL != token) {
//     lastToken = token;
//     token = strtok(NULL, "/");
//   }
//   command = lastToken;

//   // Brightness -------------------------
//   if (0 == strcmp(command, SUB_TOPIC_BRIGHTNESS)) {
//     g_displayBacklightBrightness = (int)atoi(payloadStr);    
//   } else
//   // Backlight -------------------------
//   if (0 == strcmp(command, SUB_TOPIC_BACKLIGHT)) {
//     g_displayBacklightState = (int)atoi(payloadStr);
//   } else
//   // flip -------------------------
//   if (0 == strcmp(command, SUB_TOPIC_FLIP)) {
//     Serial.println("flipping the display");
//     if (g_orientation == DISPLAY_ORIENTATION_0) {
//       g_orientation = DISPLAY_ORIENTATION_180;
//     } else {
//       g_orientation = DISPLAY_ORIENTATION_0;
//     }
//   } else
//   // temp calibration -------------------------
//   if (0 == strcmp(command, SUB_TOPIC_TEMP_CALIB)) {
//     float value = (float)atof(payloadStr);
//     envMonConfig.setTemperatureCalibrationValue(value);
//   } else
//   // humidity calibration -------------------------
//   if (0 == strcmp(command, SUB_TOPIC_HUMIDITY_CALIB)) {
//     Serial.println("Adjusting humidity calibration");
//     float value = (float)atof(payloadStr);
//     envMonConfig.setHumidityCalibrationValue(value);
//   } else
//   // Neo Status LED brightness -------------------------
//   if (0 == strcmp(command, SUB_TOPIC_LED_BRIGHTNESS)) {
// //    Serial.println("Adjusting led brightness");
//     int value = (int)atoi(payloadStr);
//     if (value >= 0 && value <= 255) {
//       g_statusLedBrightness = value;
//     } else {
//       Serial.printf("Invalid value for orientation: %d\n", value);
//     }
//   } else 
//   // Neo Status LED colour -------------------------
//   if (0 == strcmp(command, SUB_TOPIC_LED_COLOUR)) {
//     Serial.println("Setting led colour");
// //    neoStatusLed.setPixelColor(1, g_statusLedColour);
// //    neoStatusLed.show();
//   } else {
//     Serial.printf("#### unhandled command [%s]####\n", command);
//   }
    
// }
