/*
  ---------------------------------------------------------------------------
  RGB Touch Mini - Spirals Example
  v1.0 Release - 02/12/2024.

  AUTHOR/LICENSE:
  Created by Seon Rozenblum - seon@unexpectedmaker.com
  Copyright 2016 License: GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html

  LINKS:
  Project home: https://rgbtouch.io
  Blog: https://unexpectedmaker.com

  PURPOSE:
  Generate a spiral pattern on the RGB Touch Mini based on the touch input.
  ---------------------------------------------------------------------------
*/

#include "RGB_Touch_Mini.h"
#include "spirals.h"

bool init_success = false;

void setup()
{
 Serial.begin(115200);

 // Needed to see iknitial serial print messages due to
 // the delay in ESP32S3 native USB initialisation once user code is running
 // Uncomment is you want to see all serial output
 // delay(3000);

 init_success = rgbtouch.initialise();
 if (init_success)
 {
  // Set the LED brightness to 200/255 - Approx 78%
  rgbtouch.set_brightness(200);
  // Set the LED fade out speed to 33
  rgbtouch.set_fade_speed(10);
 }
}

void loop()
{
 // If the device failed to initialise/Users/seon/Documents/Arduino/libraries/RGB_Touch_Mini/RGB_Touch_Mini.cpp
 // Lock out the code below this section
 if (!init_success)
 {
  rgbtouch.fill_screen(100, 0, 0);
  return;
 }

 // Update the Audio subsystem to to play selected clips
 // If we are playing, exit early.
 if (rgbtouch.update_audio())
  return;

 // Process the touch panel input for this frame
 // If there were touches this frame, we can grab the first touch, get it's position,
 // and generate a spiral pattern around it.
 // We also play a beep to acknowledge the touch.
 if (rgbtouch.process_input() > 0)
 {
  rgbtouch.delay_next_touch(500);
  Touch pos = rgbtouch.get_touch(0);
  spirals.generate_spiral(pos.x, pos.y, 20);
  rgbtouch.play_menu_beep(5);
 }

 // We only want to generate the spiral pattern if one has been generated
 if (spirals.has_spiral())
 {
  // every time step, we grab the next LED in the spiral pattern and set it to the current touch color
  if (millis() - spirals.last_update_time > spirals.cycle_update_delay)
  {
   spirals.last_update_time = millis();
   rgbtouch.set_pixel_by_index(spirals.get_next_path_step(), FastColor(rgbtouch.cycle_touch_color()));
  }
 }

 // We update the display visuals here
 rgbtouch.update();
}
