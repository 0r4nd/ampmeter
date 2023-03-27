
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h> // uint8_t, int32_t etc


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// project libs
#include "Acs712.h"
#include "TimeInterval.h"


#define SCREEN_WIDTH 128 // OLED display width
#define SCREEN_HEIGHT 32 // OLED display height

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


/*////////////////////////// application ///////////////////////////*/
void app_timers(TimeInterval *pti, float cur) {
  uint32_t dt = pti->curTime - pti->prevTime;
  pti->timers[0] += dt * TimeInterval_timerStatus(pti,0);
  pti->timers[1] += dt * TimeInterval_timerStatus(pti,1);
  
  // current
  if (cur > 0) {
    // start timer 1
    if (TimeInterval_timerStatus(pti,1) == 0) {
      TimeInterval_timerOn(pti,1);
      pti->timers[1] = 0;
    }
    TimeInterval_timerOff(pti,0); // reset timer 0
    
  // no current
  } else {

    // reset timer 0 if:
    // - timer 0 is off
    // - timer 1 is on
    if (TimeInterval_timerStatus(pti,0)==0 && TimeInterval_timerStatus(pti,1)) {
      TimeInterval_timerOn(pti,0);
      pti->timers[0] = pti->timers[1]%1000; // fait coincider les timers
    }
    
    if (pti->timers[0] >= 6000) { // 5 seconds
      TimeInterval_timerOff(pti,0);
      TimeInterval_timerOff(pti,1);
    }
  }
}

void app_display(TimeInterval *pti, Acs712 *pacs) {
  static char buffer_str[20];
  static char amps_str[12];
  static char watts_str[12];
  float cur = pacs->current_average;

  display.clearDisplay();
  display.setTextSize(2);              // pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);             // Start at top-left corner

  // Amps
  if (cur < 10) dtostrf(cur, 1, 3, buffer_str);
  else dtostrf(cur, 1, 2, buffer_str);
  sprintf(amps_str, "%sA", buffer_str);
  display.println(amps_str);

  // Watt label
  display.setTextSize(1);
  display.setCursor(104, 0);
  display.println("Watt");

  // 3.3v
  display.setCursor(80, 9);
  sprintf(watts_str, "3v3:%s", watt_dtostrf(cur * 3.3, buffer_str));
  display.println(watts_str);

  // 5v
  display.setCursor(80, 17);
  sprintf(watts_str, "5v: %s", watt_dtostrf(cur * 5, buffer_str));
  display.println(watts_str);

  // 12v
  display.setCursor(80, 25);
  sprintf(watts_str, "12v:%s", watt_dtostrf(cur * 12, buffer_str));
  display.println(watts_str);

  // mA/h
  display.setCursor(0, 17);
  sprintf(amps_str, "%smAh", watt_dtostrf(pacs->mAh_per_hour, buffer_str));
  display.println(amps_str);
  
  // timer
  display.setCursor(0, 25);
  display.println(ms_format_time(pti->timers[1], buffer_str));
  //display.println((pacs->mAh_per_hour * 12.0)/1000.0); // w/hour

  display.display();
}

void app_process(TimeInterval *pti, Acs712 *pacs) {
  uint32_t dt = pti->curTime - pti->prevTime;
  float current_mA = pacs->current_average * 1000.0;
  pacs->mAh_per_hour += current_mA * (dt / 3600000.0);
}


// instances
TimeInterval inter = {
  .interval =     200, // 200ms
  .curTime =      0,
  .prevTime =     0,
  .frames =       0,
  .timers_flags = (0<<0) | (0<<1),
  .timers =       {0, 0},
};

Acs712 acs = {
  .zero_offset =     0.0,
  .v_per_amp =       0.066, // versions: 5A=0.185v/amp, 20A=0.100v/amp, 30A=0.066v/amp
  .vref =            5.0,
  .current_samples = 0.0,
  .current_average = 0.0,
  .mAh_per_hour =    0.0,
  .steps =          1023,
  .pin =              A0,
};

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while(1){} // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(SSD1306_WHITE);
  display.println("RESET");
  display.display();
   
  delay(500);
  Acs712_setOffset(&acs);
  TimeInterval_init(&inter);
}


void loop() {
  inter.frames++;
  inter.curTime = millis();

  if ((inter.curTime - inter.prevTime) > inter.interval) {
    Acs712_getCurrent(&acs, inter.frames, true);
    
    app_timers(&inter, acs.current_average);
    app_process(&inter, &acs);
    app_display(&inter, &acs);

    inter.frames = 0;
    inter.prevTime = inter.curTime;
    Acs712_init(&acs);
  }

  Acs712_update(&acs);
}
