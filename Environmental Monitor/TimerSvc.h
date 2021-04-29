#ifndef TIMER_SVC_H
#define TIMER_SVC_H

#include <esp32-hal-timer.h>

struct TimerNodes{
    TimerNode* next;
    TimerNode* prev;
    int period;    // millis
    bool repeat;
    int time_left; // millis
    int time_left_timestamp; // millis
};

typedef struct TimerNodes TimerNode;

class TimerSvc {
    public:
        TimerSvc();
        void begin();
        void loop();
        
        TimerNode* addTimer(int, bool);
        TimerNode* addTimerAndStart(int);
        void removeTimer(TimerNode*);
        void startTimer(TimerNode*);
        void stopTimer(TimerNode*);
        void stopTimerAndRemove(TimerNode*);
    private:
        void addTimerToActiveList(TimerNode*);
        int timeTillNextTimer();
        void timerSvcCallback();
        void sortTimers();
        void timerSleepTimeReset();
        int getTimeTillNextTimer();
        void updateActiveTimers();
};


#endif