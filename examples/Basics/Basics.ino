/*
  ---------------------------------------------------------------------------
  RGB Touch Mini - Basic Example
  v1.0 Release - 02/12/2024.

  AUTHOR/LICENSE:
  Created by Seon Rozenblum - seon@unexpectedmaker.com
  Copyright 2016 License: GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html

  LINKS:
  Project home: https://rgbtouch.io
  Blog: https://unexpectedmaker.com

  PURPOSE:
  This project shows the basic usage of the RGB Touch Mini library. It sets up the
  library, sets the LED brightness and fade out speed, and then enters a loop that
  updates the display visuals and processes button presses.
  ---------------------------------------------------------------------------
*/

#include "RGB_Touch_Mini.h"

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
  // Set the LED fade out speed to 20
  rgbtouch.set_fade_speed(20);
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
 // If there were touches this frame, we can process buttons
 if (rgbtouch.process_input() > 0)
 {
  // Example button press
  if (rgbtouch.pressed(0, 0))
  {
   rgbtouch.play_menu_beep(0);
   rgbtouch.fill_screen(0, 0, 200);
   // Add some other code here or call some other function
  }
  else if (rgbtouch.pressed(11, 11))
  {
   rgbtouch.play_menu_beep(11);
   rgbtouch.fill_screen(0, 200, 0);
   // Add some other code here or call some other function
  }
  // Check for a press within the area of (5,5) to (7,7)
  else if (rgbtouch.pressed_area(5, 5, 2, 2))
  {
   rgbtouch.play_menu_beep(5);
   rgbtouch.matrix_access()->drawCircle(5, 5, 5, RGBB(200, 0, 200));
   // Add some other code here or call some other function
  }
 }
 // We update the display visuals here
 rgbtouch.update();
}
