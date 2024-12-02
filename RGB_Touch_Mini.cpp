#include "RGB_Touch_Mini.h"

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap_col = Adafruit_MPR121();
Adafruit_MPR121 cap_row = Adafruit_MPR121();

// Local non-instance scope vars for Audio
float hhz = 440;
float tm = 1.0;
float vol = 0.5;

bool RGBTouchMini::initialise()
{

	// Setup any IO required
	pinMode(MPR_INT, INPUT_PULLUP);
	pinMode(IMU_INT, INPUT_PULLUP);
	pinMode(VBUS_SENSE, INPUT);

	// Initialise Matrix Display
	_matrix = new FastLED_NeoMatrix(leds, 12, 12, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE);
	FastLED.addLeds<NEOPIXEL, MATRIX_DATA>(leds, 144).setCorrection(TypicalLEDStrip);

	pinMode(MATRIX_PWR, OUTPUT);
	set_display_power(true);

	_matrix->begin();
	_matrix->setTextWrap(false);
	_matrix->setBrightness(255);
	// _matrix->setFont(&TomThumb);
	_matrix->setRotation(0);
	_matrix->fillScreen(0);
	_matrix->show();

	// Initialise LittleFS
	if (!LittleFS.begin(true))
	{
		Serial.println("ERROR: LittleFS failed to initialise");
		return false;
	}

	// Init MPR121 for ROWS
	if (!cap_row.begin(0x5B))
	{
		Serial.println("ERROR: MPR121 for ROWS not found, check wiring?");
		fill_screen(100, 0, 0);
		return false;
	}

	delay(100);

	// Init MPR121 for COLS
	if (!cap_col.begin(0x5A))
	{
		Serial.println("ERROR: MPR121 for COLUMNS not found, check wiring?");
		fill_screen(100, 0, 0);
		return false;
	}
	Serial.println("MPR121 COLUMNS & ROWS found!");

	cap_col.setThresholds(1, 0);
	cap_row.setThresholds(1, 0);

	// Setup Audio
	out = new AudioOutputI2S();
	wav = new AudioGeneratorWAV();

	out->SetPinout(AMP_BLCK, AMP_LRCLK, AMP_DATA);

	pinMode(AMP_SD_MODE, OUTPUT);
	digitalWrite(AMP_SD_MODE, HIGH);

	// Flash colour on the screen to show we have booted
	rgbtouch.fill_screen(200, 0, 200);
	return true;
}

/*
Display Functions
*/

FastLED_NeoMatrix *RGBTouchMini::matrix_access()
{
	return _matrix;
}

void RGBTouchMini::fill_screen(uint8_t r, uint8_t g, uint8_t b)
{
	_matrix->fillScreen(_matrix->Color(r, g, b));
	show();
}

void RGBTouchMini::clear_screen(bool _show)
{
	_matrix->clear();
	if (_show)
		show();
}

void RGBTouchMini::set_display_power(bool state)
{
	if (display_powered != state)
	{
		digitalWrite(MATRIX_PWR, (state ? HIGH : LOW));
		display_powered = state;
	}
}

// Fade speed is is between 0 and 100
void RGBTouchMini::set_fade_speed(uint8_t val)
{
	fade_speed - constrain(val, 0, 100);
}

void RGBTouchMini::set_brightness(uint8_t val)
{
	val = constrain(val, 0, 255);
	_matrix->setBrightness(val);
}

void RGBTouchMini::show()
{
	_matrix->show();
}

void RGBTouchMini::update()
{
	if (millis() - next_redraw < 25)
		return;

	next_redraw = millis();

	fadeToBlackBy(leds, NUM_LEDS, fade_speed);
	_matrix->show();
}

void RGBTouchMini::show_text(uint8_t x, uint8_t y, String txt)
{
	_matrix->setTextSize(1);
	_matrix->setCursor(x, y);
	_matrix->setTextColor(_matrix->Color(0, 0, 255));
	_matrix->print(txt);
	_matrix->show();

	Serial.println(txt);

	delay(2000);
}

uint8_t RGBTouchMini::cycle_touch_color()
{
	uint8_t col = current_touch_color;
	current_touch_color++;
	return col;
}

void RGBTouchMini::set_pixel_by_index(uint8_t index, uint32_t c)
{
	leds[index] = c;
}

// void RGBTouchMini::draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t c)
// {
//   _matrix->drawLine(x1, y1, x2, y2, c);
// }

// void RGBTouchMini::draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t c, bool filled)
// {

// }

/*
Touch Functions
*/

Touch RGBTouchMini::get_touch(uint8_t index)
{
	if (index < touches.size())
	{
		return touches[index];
	}

	return Touch(5, 5);
}

void RGBTouchMini::delay_next_touch(int period)
{
	next_touch_delta = period;
}

int RGBTouchMini::process_input()
{

	// Return no touches if timer is too quick
	if (millis() - next_touch < next_touch_delta)
	{
		return 0;
	}

	next_touch_delta = 10;
	next_touch = millis();

	first_touch = true;
	if (touches.size() > 0)
	{
		first_touch = false;
		touches.clear();
	}

	// Capture the touch matrix state
	currtouched_cols = cap_col.touched();
	currtouched_rows = cap_row.touched();

	if (currtouched_cols == 0 || currtouched_rows == 0)
	{
		return false;
	}

	for (int i = 0; i < MATRIX_SIZE; ++i)
	{
		if (readColumn(i)) // Matrix X axis
		{
			for (int j = 0; j < MATRIX_SIZE; ++j)
			{
				if (readRow(j)) // Matrix Y axis
				{
					// Serial.printf("touched (%d, %d) - count %d\n", i, j, touch_count);
					_matrix->drawPixel(i, j, ColorHUE(current_touch_color));
					// _matrix->drawPixel(i, j, CHSV(50, 255, 255));

					touches.push_back(Touch(i, j));

					current_touch_color++;
				}
			}
		}
	}

	return touches.size();
}

bool RGBTouchMini::pressed(uint8_t x, uint8_t y)
{
	// If there are no touches or it's not the first touch we exit early
	if (touches.size() == 0 || !first_touch)
		return false;

	for (size_t i = 0; i < touches.size(); i++)
	{
		if (touches[i].check(x, y))
		{
			return true;
		}
	}

	return false;
}

bool RGBTouchMini::pressed_area(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	// If there are no touches or it's not the first touch we exit early
	if (touches.size() == 0 || !first_touch)
		return false;

	for (size_t i = 0; i < touches.size(); i++)
	{
		if (touches[i].check_bounds(x, y, w, h))
		{
			return true;
		}
	}

	return false;
}

bool RGBTouchMini::readColumn(int x)
{
	return (currtouched_cols & (1 << x));
}

bool RGBTouchMini::readRow(int y)
{
	return (currtouched_rows & (1 << y));
}

/*
AUDIO
*/

bool RGBTouchMini::update_audio()
{
	if (wav->isRunning())
	{
		if (!wav->loop())
			wav->stop();

		// If we are playing a clip, return true
		// But add a 500ms pause so the button press doesn't repeat immediately
		delay_next_touch(500);
		return true;
	}

	// no clip being played, so retun false;
	return false;
}

void RGBTouchMini::set_volume(uint8_t vol)
{
	out->SetGain((float)vol / max_volume);
}

float RGBTouchMini::sine_wave(const float time)
{
	float v = cos(TWO_PI * hhz * time * tm);
	v *= vol;
	vol = constrain(vol - 0.0001, 0.00, 1.0); // Fade down over time
	return v;
}

void RGBTouchMini::play_menu_beep(int index)
{
	hhz = piano_notes[index];
	tm = 1.0;

	// Playing this tone via a function doesn't seem to respect the I2S Amps volume settings, so we compensate here
	// It seems 1/6 of the volume % seems about right
	// vol = (max_volume / 100.0);
	vol = 0.05;
	if (wav->isRunning())
		wav->stop();
	func = new AudioFileSourceFunction(0.25);
	func->addAudioGenerators([this](const float time)
							 { return sine_wave(time); });
	wav->begin(func, out);
}

RGBTouchMini rgbtouch;