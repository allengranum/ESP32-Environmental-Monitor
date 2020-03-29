//#include "esp_mqtt.h"
#include "hardwareSerial.h"
#include <PubSubClient.h>

extern char* g_mqttTopicDevice;

boolean mqttConnect(PubSubClient* client, char* broker, void (*callback)(char*, uint8_t*, unsigned int), const char* deviceName) {
  boolean retVal = false;
  
  if (broker) {
    Serial.printf("mqttConnect: Connecting to mqttBroker = [%s]\n", broker);  
//  delay(2000);
    client->setServer(broker, 1883);
    client->setCallback(callback);
//    Serial.printf("Attempting to connect to MQTT Broker [%s]\n", deviceName);
//    delay(2000);
    if (client->connect(deviceName)) {
      Serial.printf("Connected to MQTT broker [%s]\n", broker);
      client->subscribe(g_mqttTopicDevice);
      Serial.printf("Subscribed to topic: [%s]\n", g_mqttTopicDevice);
    } else {
      Serial.printf("mqttConnect: ERROR: Failed to connect to MQTT broker [%s]\n", broker);
    }

    if (client->connected()) {
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
