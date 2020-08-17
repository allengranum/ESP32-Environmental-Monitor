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