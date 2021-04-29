#include "DeviceInfo.h"

DeviceInfo::DeviceInfo() {
}

EnvMonScreen  DeviceInfo::getScreen(){
    return screen;
}
void DeviceInfo::setScreen(EnvMonScreen newScreen) {
    screen = newScreen;
}

int DeviceInfo::getLightValue() {
    return light;
}

void DeviceInfo::setLightValue(int lightValue) {
    light = lightValue;
}

bool  DeviceInfo::motionDetected(){
    return currentMotionDetectedState;
}

void DeviceInfo::setMotionDetected(bool motion) {
    currentMotionDetectedState = motion;
}