
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



char *ms_format_time(uint32_t ms, char *dst) {
  uint32_t s = ms / 1000;
  uint32_t m = s / 60;
  uint32_t h = m / 60;
  ms %= 1000;
  s %= 60;
  m %= 60;
  sprintf(dst, "%02lu:%02lu:%02lu", h, m, s);
  return dst;
}

char *watt_dtostrf(float watts, char *str) {
  if (watts < 10) dtostrf(watts, 1, 2, str);
  else if (watts < 100) dtostrf(watts, 1, 1, str);
  else dtostrf(watts, 1, 0, str);
  return str;
}


/*////////////////////////// application ///////////////////////////*/
void app_timers(TimeInterval &ti, float cur) {
  uint32_t dt = ti.getCurTime() - ti.getPrevTime();
  ti.setTimer(0, ti.getTimer(0) + dt * ti.getTimerStatus(0));
  ti.setTimer(1, ti.getTimer(1) + dt * ti.getTimerStatus(1));
  
  // current
  if (cur > 0) {
    // start timer 1
    if (ti.getTimerStatus(1) == 0) {
      ti.setTimerOn(1);
      ti.setTimer(1, 0); // set timer 1 to 0
    }
    ti.setTimerOff(0); // reset timer 0
    
  // no current
  } else {

    // reset timer 0 if:
    // - timer 0 is off
    // - timer 1 is on
    if (ti.getTimerStatus(0)==0 && ti.getTimerStatus(1)) {
      ti.setTimerOn(0);
      ti.setTimer(0, ti.getTimer(1)%1000); // fait coincider les timers
    }
    
    if (ti.getTimer(0) >= 6000) { // 5 seconds
      ti.setTimerOff(0);
      ti.setTimerOff(1);
    }
  }
}

void app_display(TimeInterval &ti, Acs712 &acs) {
  static char buffer_str[20];
  static char amps_str[12];
  static char watts_str[12];
  float cur = acs.getCurrentAverage();

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
  sprintf(amps_str, "%smAh", watt_dtostrf(acs.getMAhPerHour(), buffer_str));
  display.println(amps_str);
  
  // timer
  display.setCursor(0, 25);
  display.println(ms_format_time(ti.getTimer(1), buffer_str));
  //display.println((pacs->mAh_per_hour * 12.0)/1000.0); // w/hour

  display.display();
}

void app_process(TimeInterval &ti, Acs712 &acs) {
  uint32_t dt = ti.getCurTime() - ti.getPrevTime();
  float current_mA = acs.getCurrentAverage() * 1000.0;
  acs.setMAhPerHour(acs.getMAhPerHour() + current_mA * (dt / 3600000.0));
}



TimeInterval ti(200);
Acs712 acs(
  0.066, // versions: 5A=0.185v/amp, 20A=0.100v/amp, 30A=0.066v/amp
  5.0,
  1023,
  A0
);


void setup() {
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while(1); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(SSD1306_WHITE);
  display.println("RESET");
  display.display();
   
  delay(500);
  acs.calibrateOffset();
  ti.init();
}


void loop() {
  ti.setFrames(ti.getFrames()+1);
  ti.setCurTime(millis());

  if ((ti.getCurTime() - ti.getPrevTime()) > ti.getInterval()) {
    acs.getCurrent(ti.getFrames(), true);
    
    app_timers(ti, acs.getCurrentAverage());
    app_process(ti, acs);
    app_display(ti, acs);

    ti.setFrames(0);
    ti.setPrevTime(ti.getCurTime());
    acs.init();
  }

  acs.update();
}
