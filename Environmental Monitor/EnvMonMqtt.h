#ifndef ENV_MON_MQTT_H
#define ENV_MON_MQTT_H

#include "environment_monitor.h"
#include "EnvMonConfig.h"

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

char g_mqttTopicDevice[128];
char g_topic_brightness[128];
char g_topic_orientation[128];
char g_topic_temp_calib[128];
char g_topic_humidity_calib[128];

class EnvMonMqtt {
    public:
        EnvMonMqtt(EnvMonConfig, DeviceInfo, WiFiClient);
        void begin();
        void init();

    private:
        void mqttCallback(char* topic, byte* payload, unsigned int length);
        void reconnect();
        void mqttConnectLocal(char* broker, void (*callback)(char*, uint8_t*, unsigned int), const char* deviceName);

        boolean          mqttConnected;
        EnvMonConfig     envMonConfig;
        DeviceInfo*      deviceInfo;
        WiFiClient*      espClient;
        PubSubClient*    mqttClient;
};

#endif 
