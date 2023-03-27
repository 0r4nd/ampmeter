
#ifndef _ACS712_H
#define _ACS712_H


typedef struct {
  float zero_offset;     // reset
  float v_per_amp;       // versions: 5A=0.185v/amp, 20A=0.100v/amp, 30A=0.066v/amp
  float vref;            // 5v
  float current_samples; // analogRead() + analogRead() + ...
  float current_average; // current_samples / total
  float mAh_per_hour;    //
  uint32_t steps;        // 1023 (arduino)
  uint32_t pin;          // A0, A1, etc
} Acs712;


#endif // _ACS712_H
