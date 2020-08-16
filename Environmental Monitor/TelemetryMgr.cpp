// This code handles the gathering of currently captured sensor
// data from the DeviceInfo object and sends it to the remote
// server at a given interval

#include "TelemetryMgr.h"

bool timeToSendData;
bool timeToSendMotionData = true;
long unsigned int noMotionDelay = 10000;
long unsigned int motionDetectedTime;

TelemetryMgr::TelemetryMgr() {
    sendTelemetry = false;
}

void TelemetryMgr::begin() {
    sendTelemetry = false;
}

void TelemetryMgr::loop() {
    // Send telemetry data if time to send;
}

void TelemetryMgr::telemetryTimerCallback() {
    if (sendTelemetry)
        timeToSendData = true;
}

void TelemetryMgr::startTelemetry() {
    sendTelemetry = true;
}

void TelemetryMgr::stopTelemetry() {
    sendTelemetry = false;
}

void TelemetryMgr::sendTelemetryData() {
  if (mqttClient.connected()) {
    // Publish
    char topic[256];
    char value[8];

    sprintf(topic, "%s/%s/temperature", MQTT_TOPIC_ROOT, g_deviceName);
    sprintf(value, "%.1f", currentTemp);
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
    sprintf(topic, "%s/%s/humidity", MQTT_TOPIC_ROOT, g_deviceName);
    sprintf(value, "%.1f", currentHumidity);
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
    sendMotionData();
    sendLightData();
  } else {
    Serial.println("MQTT broker not connected. Can't publish data");
    g_mqttConnected = false;
  }
}

void TelemetryMgr::sendLightData() {
  if (mqttClient.connected()) {
    char topic[256];
    char value[8];
    sprintf(topic, "%s/%s/light", MQTT_TOPIC_ROOT, g_deviceName);
    sprintf(value, "%d", analogRead(LIGHT_SENSOR_PIN));
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
  } else {
    Serial.println("MQTT broker not connected. Can't publish light data");
    g_mqttConnected = false;
  }
}

void TelemetryMgr::sendStatusLedBrightness() {
  if (mqttClient.connected()) {
    char topic[256];
    char value[8];
    sprintf(topic, "%s/%s/%s", MQTT_TOPIC_ROOT, g_deviceName, SUB_TOPIC_LED_BRIGHTNESS);
    sprintf(value, "%d", value);
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
  } else {
    Serial.println("MQTT broker not connected. Can't publish status LED brightness data");
    g_mqttConnected = false;
  }
}

void TelemetryMgr::sendMotionData() {
  if (mqttClient.connected()) {
    // Publish
    char topic[256];
    char value[8];
    sprintf(topic, "%s/%s/motion", MQTT_TOPIC_ROOT, g_deviceName);
    sprintf(value, "%d", (currentMotionDetectedState ? 1 : 0)); // these relate to colours in IOT-ON-OFF app
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
//    Serial.printf("Motion: %d\n", currentMotionDetectedState);
  } else {
    Serial.println("MQTT broker not connected. Can't publish motion data");
    g_mqttConnected = false;
  }
}

void TelemetryMgr::sendTemperatureCalibrationValue() {
  if (mqttClient.connected()) {
    char topic[256];
    char value[6];
    sprintf(topic, "%s/%s/%s", MQTT_TOPIC_ROOT, g_deviceName, SUB_TOPIC_TEMP_CALIB);
    sprintf(value, "%.1f", g_temperatureCalibrationValue);
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
  } else {
    Serial.println("MQTT broker not connected. Can't publish temperature calibration data");
    g_mqttConnected = false;
  }
}

void TelemetryMgr::sendHumidityCalibrationValue() {
  if (mqttClient.connected()) {
    char topic[256];
    char value[6];
    sprintf(topic, "%s/%s/%s", MQTT_TOPIC_ROOT, g_deviceName, SUB_TOPIC_HUMIDITY_CALIB);
    sprintf(value, "%.1f", g_humidityCalibrationValue);
    Serial.printf("Publish: %s:%s\n", topic, value);
    mqttClient.publish(&topic[0], value, true);
  } else {
    Serial.println("MQTT broker not connected. Can't publish humidity calibration data");
    g_mqttConnected = false;
  }
}


void TelemetryMgr::loop() {

}

// THIS CODE NEEDS TO MOVE INTO TELEMETRY MGR AND USE THE TIMER SERVICE
  //
  // // Set up the data collection timer
  //   timeToSendData = true;

  //   // Use 1st timer of 4 (counted from zero).
  //   // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  //   // info).
  //   dataCollectionTimer = timerBegin(0, 80, true);
  
  //   // Attach callback function to our timer.
  //   timerAttachInterrupt(dataCollectionTimer, &dataCollectionTimerCallback, true);
  
  //   // Set alarm to call onTimer function every second (value in microseconds).
  //   // Repeat the alarm (third parameter)
  //   timerAlarmWrite(dataCollectionTimer, (getDataPublishInterval() * 1000000), true);
  
  //   // Start an alarm
  //   timerAlarmEnable(dataCollectionTimer);