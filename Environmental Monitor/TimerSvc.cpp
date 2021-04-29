#include "TimerSvc.h"

hw_timer_t * espTimer0 = NULL;

TimerNode* idelTimerList;
TimerNode* activeTimerList;

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// used for synchronizing access to active timers with ISR
//portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

int currentTimerSnapshotTimeLeft;
int currentTimerSnapshotTime;
int currentTimerTotalTime;

void IRAM_ATTR hwTimerCallback()
{
}

TimerSvc::TimerSvc()
{
}

void TimerSvc::timerSvcCallback()
{
    // Start at the front of the list and process until 
}

void TimerSvc::begin()
{
    espTimer0 = timerBegin(0, 80, true);
    timerAttachInterrupt(espTimer0, &hwTimerCallback, true);
}

void TimerSvc::loop()
{
}

TimerNode* TimerSvc::addTimer(int timerTimeMs, bool repeat)
{
    // add timer to list
    TimerNode* newTimer = (TimerNode*)malloc(sizeof(TimerNode));

    if (NULL == newTimer) {
        return NULL;
    }

    newTimer->period = timerTimeMs;
    newTimer->repeat = repeat;

    // add node to list
    if (!idelTimerList)
        idelTimerList = newTimer;
    else {
        TimerNode* currentTimer = idelTimerList;
        while (currentTimer->next) {
            currentTimer = currentTimer->next;
        }
        currentTimer->next = newTimer;
        newTimer->prev = currentTimer;
    }

    return newTimer;
}

void TimerSvc::startTimer(TimerNode* timer)
{
    addTimerToActiveList(timer);
}

void TimerSvc::removeTimer(TimerNode* timerNode)
{}

void TimerSvc::stopTimer(TimerNode* timerNode)
{

}

int TimerSvc::timeTillNextTimer()
{

}

void TimerSvc::sortTimers()
{

}

void updateActiveTimers()
{
    ActiveTimerNode* currentTimer;
    if (NULL == activeTimerList) {
        return;
    } else {
        currentTimer = activeTimerList;
        while (currentTimer) {
            
        }
    }
}

/*/////////////  P R O T E C T E D  \\\\\\\\\\\\\\\\\*/
void TimerSvc::addTimerToActiveList(TimerNode* timerToInsert)
{
    TimerNode* currentTimer;
    TimerNode* nextTimer;
    bool resetTimerFlag = false;


    updateActiveTimers(); // To make sure the "time left" is updated

    if (NULL == activeTimerList) {
        activeTimerList = timerToInsert;
        // TODO : start the hardware timer here !!!
        timerAlarmWrite(espTimer0, timerToInsert->time_left * 1000, true);
        timerAlarmEnable(espTimer0);
        timerToInsert->time_left_timestamp = millis();
        return;
    } else {
        currentTimer = activeTimerList;

        while (currentTimer->next && (currentTimer->time_left <= timerToInsert->time_left)) {
            currentTimer = currentTimer->next;
        }
        
        // Insert the active timer in the list before the current timer
            
        // First save the currentTimer's value of prev
        TimerNode* prevCurrentTimer = currentTimer->prev;

        // Insert the new node in front of the current timer
        currentTimer->prev = timerToInsert;
        timerToInsert->next = currentTimer;

        // and also insert it after the prev of the currentTimer
        if (prevCurrentTimer) {
            prevCurrentTimer->next = timerToInsert;
            timerToInsert->prev = prevCurrentTimer;
        }

        // Check to see if we need to rest the hardwae timer
        if (getTimeTillNextTimer() > timerToInsert->time_left)
            timerSleepTimeReset();
    }
}

void TimerSvc::timerSleepTimeReset()
{
    int retVal = 0;
    int timeLeftForCurrentTimer = 0; 
    timeLeftForCurrentTimer = getTimeTillNextTimer();

        // stop the current hw_timer
        timerStop(espTimer0);

        // start the hw_timer with the new value
        timerAlarmWrite(espTimer0, timeLeftForCurrentTimer, true);
        timerAlarmEnable(timer);

        // update the time left on all timers in the list
}

void TimerSvc::updateActiveTimers() {
    TimerNode* currentTimer = NULL;
    int now = 0;
    int timeToSubtract = 0;

    currentTimer = activeTimerList;
    while(currentTimer) {
        now = millis();
        timeToSubtract = now - currentTimer->time_left_timestamp;
        currentTimer->time_left = currentTimer->time_left - timeToSubtract;
        currentTimer->time_left_timestamp = now;
        if (currentTimer->time_left<0)
            currentTimer->time_left = 0;
        currentTimer = currentTimer->next;
    }
}

int TimerSvc::getTimeTillNextTimer() {
    int runDuration = millis() - currentTimerSnapshotTime; // ??? wtf ???
    return currentTimerSnapshotTimeLeft - runDuration;
}