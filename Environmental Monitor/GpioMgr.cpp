#include "GpioMgr.h"
#include "EspPins.h"
#include <Wire.h>
#include <DHT.h>

float g_temperatureCalibrationValue = 0;
float g_humidityCalibrationValue = 0;

DHT dht(DHT_SENSOR_PIN, DHTTYPE);


GpioMgr::GpioMgr() {    
    
}

void GpioMgr::begin() {
    dht.begin();
    Wire.begin();
}