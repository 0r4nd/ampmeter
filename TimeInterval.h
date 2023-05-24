
#ifndef TIMEINTERVAL_H_
#define TIMEINTERVAL_H_

#include <Arduino.h>
#include <stdint.h>


class TimeInterval {
  public:
    TimeInterval(uint32_t interval);
    void init();
    
    void setTimerOn(uint32_t channel);
    void setTimerOff(uint32_t channel);
    uint32_t getTimerStatus(uint32_t channel);

    uint32_t getCurTime();
    void setCurTime(uint32_t curTime);
    uint32_t getPrevTime();
    void setPrevTime(uint32_t prevTime);
    uint32_t getInterval();
    void setInterval(uint32_t interval);
    uint32_t getFrames();
    void setFrames(uint32_t frames);

    uint32_t getTimer(uint32_t channel);
    void setTimer(uint32_t channel, uint32_t value);

  private:
    static constexpr int MAX_TIMERS = 32; // can't exceed 32
    uint32_t interval;     // interval before interupt
    uint32_t curTime;      // current time (in ms)
    uint32_t prevTime;     // previous time (at interupt)
    uint32_t frames;       // frames passed between interuption
    uint32_t timers_flags; // bits for timers usage
    uint32_t timers[MAX_TIMERS];   // generic timers
};



#endif // TIMEINTERVAL_H_
