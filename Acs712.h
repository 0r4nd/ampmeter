
#ifndef ACS712_H_
#define ACS712_H_

#include <Arduino.h>
#include <stdint.h>
#include <math.h>

class Acs712 {
  public:
    Acs712(float v_per_amp, float vref, uint32_t steps, uint32_t pin);
    void init();
    void update();
    void calibrateOffset();
    float getCurrent(uint32_t sample_count, bool absValue);
    float getCurrentAverage();
    float getMAhPerHour();
    void setMAhPerHour(float mAh_per_hour);

  private:
    float zero_offset;     // reset
    float v_per_amp;       // versions: 5A=0.185v/amp, 20A=0.100v/amp, 30A=0.066v/amp
    float vref;            // 5v
    float current_samples; // analogRead() + analogRead() + ...
    float current_average; // current_samples / total
    float mAh_per_hour;    //
    uint32_t steps;        // 1023 (arduino)
    uint32_t pin;          // A0, A1, etc
};

#endif // ACS712_H_
