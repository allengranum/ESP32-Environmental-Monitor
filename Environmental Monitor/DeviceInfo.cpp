#include "DeviceInfo.h"

DeviceInfo::DeviceInfo() {
}

EnvMonScreen  DeviceInfo::getScreen(){
    return screen;
}
void DeviceInfo::setScreen(EnvMonScreen newScreen) {
    screen = newScreen;
}