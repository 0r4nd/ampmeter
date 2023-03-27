
#include "Acs712.h"

float Acs712_init(Acs712 *p) {
  p->current_samples = 0;
}


void Acs712_update(Acs712 *p) {
  p->current_samples += analogRead(p->pin);
}


void Acs712_setOffset(Acs712 *p) {
  uint32_t sample_count = 150;
  Acs712_init(p);
  for (int i = 0; i < sample_count; i++) {
    Acs712_update(p);
    delay(3);
  }
  p->zero_offset = p->current_samples / sample_count;
  p->current_samples = 0;
}


float Acs712_getCurrent(Acs712 *p, uint32_t sample_count, bool absValue) {
  float avg = p->current_samples / sample_count;
  avg = (avg - p->zero_offset) * (p->vref / p->steps);
  avg = avg / p->v_per_amp;
  avg = absValue? fabs(avg) : avg;
  if (avg < 0.02 || avg > 27) avg = 0;
  p->current_average = avg;
  return avg;
}
