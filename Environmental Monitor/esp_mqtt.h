#ifndef ESP_MQTT_H
#define ESP_MQTT_H

//#include <stdint.h>
//#include <PubSubClient.h>

char* g_mqttTopicRoot;
char g_mqttTopicDevice[128];
char g_topic_brightness[128];
char g_topic_orientation[128];
char g_topic_temp_calib[128];
char g_topic_humidity_calib[128];


#define MQTT_TOPIC_ROOT          "/granumIot/granumHome"
#define SUB_TOPIC_BRIGHTNESS     "brightness"
#define SUB_TOPIC_BACKLIGHT      "backlight"
#define SUB_TOPIC_ORIENTATION    "orientation"
#define SUB_TOPIC_FLIP           "flip"
#define SUB_TOPIC_TEMP_CALIB     "t_calib"
#define SUB_TOPIC_HUMIDITY_CALIB "h_calib"
#define SUB_TOPIC_LED_BRIGHTNESS "led_brightness"
#define SUB_TOPIC_LED_COLOUR     "led_colour"
#define SUB_TOPIC_CONNECTED      "connected"



//boolean mqttConnect(PubSubClient* client, char* broker, void (*callback)(char*, uint8_t*, unsigned int), const char* deviceName);

#endif
