
#include "TimeInterval.h"


TimeInterval::TimeInterval(uint32_t interval = 200) {
  this->interval =     interval;
  this->curTime =      0;
  this->prevTime =     0;
  this->frames =       0;
  this->timers_flags = 0;
  for (int i = 0; i < MAX_TIMERS; i++) {
    this->timers_flags = (0<<i);
    this->timers[i] = 0;
  }
}

void TimeInterval::init() {
  curTime = millis();
  prevTime = curTime;
}

void TimeInterval::setTimerOn(uint32_t channel) {
  timers_flags |= 1 << channel;
}

void TimeInterval::setTimerOff(uint32_t channel) {
  timers_flags &= ~(1 << channel);
  timers[channel] = 0;
}

uint32_t TimeInterval::getTimerStatus(uint32_t channel) {
  return (timers_flags & (1 << channel)) >> channel;
}

uint32_t TimeInterval::getCurTime() { return curTime; }
void TimeInterval::setCurTime(uint32_t curTime) { this->curTime = curTime; }

uint32_t TimeInterval::getPrevTime() { return prevTime; }
void TimeInterval::setPrevTime(uint32_t prevTime) { this->prevTime = prevTime; }

uint32_t TimeInterval::getInterval() { return interval; }
void TimeInterval::setInterval(uint32_t interval) { this->interval = interval; }

uint32_t TimeInterval::getFrames() { return frames; }
void TimeInterval::setFrames(uint32_t frames) { this->frames = frames; }

uint32_t TimeInterval::getTimer(uint32_t channel) { return timers[channel]; }
void TimeInterval::setTimer(uint32_t channel, uint32_t value) { this->timers[channel] = value; }
