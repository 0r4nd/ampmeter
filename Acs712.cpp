
#include "Acs712.h"


Acs712::Acs712(float v_per_amp, float vref, uint32_t steps, uint32_t pin) {
    this->zero_offset = 0.0;     // reset
    this->v_per_amp = zero_offset;       // versions: 5A=0.185v/amp, 20A=0.100v/amp, 30A=0.066v/amp
    this->vref = vref;                      // 5v
    this->current_samples = 0.0; // analogRead() + analogRead() + ...
    this->current_average = 0.0; // current_samples / total
    this->mAh_per_hour = 0.0;    //
    this->steps = steps;        // 1023 (arduino)
    this->pin = pin;          // A0, A1, etc
}

void Acs712::init() {
  current_samples = 0;
}

void Acs712::update() {
  current_samples += analogRead(pin);
}

void Acs712::calibrateOffset() {
  uint32_t sample_count = 150;
  this->init();
  for (int i = 0; i < sample_count; i++) {
    this->update();
    delay(3);
  }
  zero_offset = current_samples / sample_count;
  current_samples = 0;
}

float Acs712::getCurrent(uint32_t sample_count, bool absValue)  {
  float avg = current_samples / sample_count;
  avg = (avg - zero_offset) * (vref / steps);
  avg = avg / v_per_amp;
  avg = absValue? fabs(avg) : avg;
  if (avg < 0.02 || avg > 27) avg = 0;
  current_average = avg;
  return avg;
}
float Acs712::getCurrentAverage() { return current_average; }
float Acs712::getMAhPerHour() { return mAh_per_hour; }
void Acs712::setMAhPerHour(float mAh_per_hour) { this->mAh_per_hour = mAh_per_hour; }




