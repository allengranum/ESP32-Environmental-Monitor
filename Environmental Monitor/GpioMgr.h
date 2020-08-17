#ifndef GPIO_MGR_H
#define GPIO_MGR_H

#include "DeviceInfo.h"

// DHT temperature stuff
#define DHTTYPE DHT22

class GpioMgr {
    public:
        GpioMgr(DeviceInfo*);
        void begin();
        void loop();
        int buttonInit();
        void readTemperature();
        void readHumidity();
        void readLight();
        void readMotion();
    private:
        void motionSensorInit();
        void readMotionData();

        DeviceInfo* deviceInfo;
};


#endif