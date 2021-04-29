#ifndef TELEMETRY_MGR_H
#define TELEMETRY_MGR_H

class TelemetryMgr {
    public:
        TelemetryMgr();
        void begin();
        void loop();
        
        void startTelemetry();
        void stopTelemetry();
        void sendTelemetryData();
        void sendLightData();
        void sendStatusLedBrightness();
        void sendMotionData();
        void sendTemperatureCalibrationValue();
        void sendHumidityCalibrationValue();
    private:
        void telemetryTimerCallback();
        void loop();
        bool sendTelemetry;
};


#endif