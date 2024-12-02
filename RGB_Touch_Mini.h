#pragma once

#include "arduino.h"
#include <map>
#include <vector>

// Touch
#include "Adafruit_MPR121.h"

// Display
#include <Adafruit_GFX.h>      // Library Manager
#include <FastLED_NeoMatrix.h> // Library Manager
#include <FastLED.h>           // Library Manager - 3.94 or later

// Audio
// Library Manager - ESP8266Audio - 1.9.7 or later
#include "AudioFileSourcePROGMEM.h"
#include "AudioFileSourceFunction.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

// IMU
// #include <Adafruit_LIS3DH.h>
// #include <Adafruit_Sensor.h>

// Storage
#include <LittleFS.h>

constexpr int NUM_LEDS = 144;

#define MATRIX_SIZE 12
#define delay FastLED.delay

// IO assignments
#define MATRIX_DATA 39
#define MATRIX_PWR 38
#define VBUS_SENSE 48
#define IMU_INT 7
#define MPR_INT 15
#define DEVICE_PWR 21
#define AMP_DATA 35
#define AMP_BLCK 36
#define AMP_LRCLK 37
#define AMP_SD_MODE 34

struct Touch
{
 uint8_t x;
 uint8_t y;

 Touch()
     : x(-1), y(-1) {}
 Touch(uint8_t px, uint8_t py)
     : x(px), y(py) {}

 bool check(uint8_t px, uint8_t py)
 {
  return (px == x && py == y);
 }

 bool check_bounds(uint8_t px, uint8_t py, uint8_t pw, uint8_t ph)
 {
  Serial.printf("%d,%d in bounds (%d, %d) - (%d, %d)\n", x, y, px, py, px + pw, py + ph);
  return (x >= px && x < px + pw && y >= py && y < py + ph);
 }
};

static uint16_t RGBB(uint8_t r, uint8_t g, uint8_t b)
{
 //   return ((r / 8) << 11) | ((g / 4) << 5) | (b / 8);
 return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
}

static CHSV ColorHUE(uint8_t hsv)
{
 uint16_t val = constrain((uint16_t)((float)hsv * 0.712), 1, 255);
 return CHSV((uint8_t)val, 255, 255);
}

static uint32_t CRGBtoUint32(CRGB color)
{
 // Combine the 8-bit R, G, B components into a 32-bit integer
 // Assuming the format is 0xRRGGBB
 return ((uint32_t)color.red << 16) | ((uint32_t)color.green << 8) | (uint32_t)color.blue;
}

static uint32_t FastColor(uint8_t hue)
{
 CHSV colHSV(hue, 255, 255);
 CRGB col;
 hsv2rgb_rainbow(colHSV, col);
 return CRGBtoUint32(col);
}

class RGBTouchMini
{

public:
 bool initialise();
 bool vbus_preset();

 // Display related functions
 FastLED_NeoMatrix *matrix_access();
 void fill_screen(uint8_t r, uint8_t g, uint8_t b);
 void clear_screen(bool show = true);
 void set_display_power(bool state);
 void set_brightness(uint8_t val);
 void set_fade_speed(uint8_t val);
 void update();
 void show();
 void show_text(uint8_t x, uint8_t y, String txt);
 uint8_t cycle_touch_color();
 void set_pixel_by_index(uint8_t index, uint32_t c);

 // Touch related functions
 int process_input();
 bool pressed(uint8_t x, uint8_t y);
 bool pressed_area(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
 void delay_next_touch(int period);
 Touch get_touch(uint8_t index);

 // Audio functions
 bool update_audio();
 void play_menu_beep(int index);
 void set_volume(uint8_t vol);
 static float sine_wave(const float time);

private:
 // Display related variables
 FastLED_NeoMatrix *_matrix;
 CRGB leds[144];
 unsigned long next_redraw = 0;
 uint8_t current_touch_color = 0;
 uint8_t fade_speed = 20;
 bool display_powered = false;

 // Touch related variables
 std::vector<Touch> touches;
 uint16_t currtouched_cols;
 uint16_t currtouched_rows;
 unsigned long next_touch = 0;
 int next_touch_delta = 10;
 bool first_touch = false;

 bool readColumn(int x);
 bool readRow(int y);

 // Audio
 AudioGeneratorWAV *wav;
 AudioFileSourcePROGMEM *file;
 AudioOutputI2S *out;
 AudioFileSourceFunction *func;
 float max_volume = 54.0;

 std::vector<float> piano_notes = {
     1046.50,
     1108.73,
     1174.66,
     1244.51,
     1318.51,
     1396.91,
     1479.98,
     1567.98,
     1661.22,
     1760.00,
     1864.66,
     1975.53};
};

extern RGBTouchMini rgbtouch;