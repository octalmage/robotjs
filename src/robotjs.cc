#include <napi.h>
#include <ctype.h>
#include <stdlib.h>
#include <vector>
#include "mouse.h"
#include "deadbeef_rand.h"
#include "keypress.h"
#include "screen.h"
#include "screengrab.h"
#include "MMBitmap.h"
#include "bitmap_find.h"
#include "color_find.h"
#include "io.h"
#include "snprintf.h"
#include "microsleep.h"
#if defined(USE_X11)
	#include "xdisplay.h"
#endif

#if !defined(ROBOTJS_HAS_PNG)
#define ROBOTJS_HAS_PNG 0
#endif

//Global delays.
int mouseDelay = 10;
int keyboardDelay = 10;

#if defined(IS_MACOSX)
static const unsigned MACOS_TYPE_STRING_DEFAULT_CPM = 1000;
#endif

/*
 __  __
|  \/  | ___  _   _ ___  ___
| |\/| |/ _ \| | | / __|/ _ \
| |  | | (_) | |_| \__ \  __/
|_|  |_|\___/ \__,_|___/\___|

*/

int CheckMouseButton(const char * const b, MMMouseButton * const button)
{
	if (!button) return -1;

	if (strcmp(b, "left") == 0)
	{
		*button = LEFT_BUTTON;
	}
	else if (strcmp(b, "right") == 0)
	{
		*button = RIGHT_BUTTON;
	}
	else if (strcmp(b, "middle") == 0)
	{
		*button = CENTER_BUTTON;
	}
	else
	{
		return -2;
	}

	return 0;
}

Napi::Value dragMouseWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if (info.Length() < 2 || info.Length() > 3)
	{
		Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}

	const int32_t x = info[0].As<Napi::Number>().Int32Value();
	const int32_t y = info[1].As<Napi::Number>().Int32Value();
	MMMouseButton button = LEFT_BUTTON;

	if (info.Length() == 3)
	{
		std::string bstr = info[2].As<Napi::String>().Utf8Value();
		const char * const b = bstr.c_str();

		switch (CheckMouseButton(b, &button))
		{
			case -1:
				Napi::Error::New(env, "Null pointer in mouse button code.").ThrowAsJavaScriptException();
return env.Null();
				break;
			case -2:
				Napi::Error::New(env, "Invalid mouse button specified.").ThrowAsJavaScriptException();
return env.Null();
				break;
		}
	}

	MMSignedPoint point;
	point = MMSignedPointMake(x, y);
	dragMouse(point, button);
	microsleep(mouseDelay);

	return Napi::Number::New(env, 1);
}

Napi::Value updateScreenMetricsWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	updateScreenMetrics();

	return Napi::Number::New(env, 1);
}

Napi::Value moveMouseWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 2)
	{
		Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}

	int32_t x = info[0].ToNumber().Int32Value();
	int32_t y = info[1].ToNumber().Int32Value();

	MMSignedPoint point;
	point = MMSignedPointMake(x, y);
	moveMouse(point);
	microsleep(mouseDelay);

	return Napi::Number::New(env, 1);
}

Napi::Value moveMouseSmoothWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if (info.Length() > 3 || info.Length() < 2 )
	{
		Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}
	size_t x = static_cast<size_t>(info[0].ToNumber().Int32Value());
	size_t y = static_cast<size_t>(info[1].ToNumber().Int32Value());

	MMPoint point;
	point = MMPointMake(x, y);
	if (info.Length() == 3)
	{
		size_t speed = static_cast<size_t>(info[2].ToNumber().Int32Value());
		smoothlyMoveMouse(point, speed);
	}
	else
	{
		smoothlyMoveMouse(point, 3.0);
	}
	microsleep(mouseDelay);

	return Napi::Number::New(env, 1);
}

Napi::Value getMousePosWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	MMSignedPoint pos = getMousePos();

	//Return object with .x and .y.
	Napi::Object obj = Napi::Object::New(env);
	obj.Set(Napi::String::New(env, "x"), Napi::Number::New(env, (int)pos.x));
	obj.Set(Napi::String::New(env, "y"), Napi::Number::New(env, (int)pos.y));
	return obj;
}

Napi::Value mouseClickWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	MMMouseButton button = LEFT_BUTTON;
	bool doubleC = false;

	if (info.Length() > 0)
	{
		std::string bstr = info[0].ToString().Utf8Value();
		const char * const b = bstr.c_str();

		switch (CheckMouseButton(b, &button))
		{
			case -1:
				Napi::Error::New(env, "Null pointer in mouse button code.").ThrowAsJavaScriptException();
return env.Null();
				break;
			case -2:
				Napi::Error::New(env, "Invalid mouse button specified.").ThrowAsJavaScriptException();
return env.Null();
				break;
		}
	}

	if (info.Length() == 2)
	{
		doubleC = info[1].ToBoolean().Value();
	}
	else if (info.Length() > 2)
	{
		Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}

	if (!doubleC)
	{
		clickMouse(button);
	}
	else
	{
		doubleClick(button);
	}

	microsleep(mouseDelay);

	return Napi::Number::New(env, 1);
}

Napi::Value mouseToggleWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	MMMouseButton button = LEFT_BUTTON;
	bool down = false;

	if (info.Length() > 0)
	{
		const char *d;

		std::string dstr = info[0].As<Napi::String>();
		d = dstr.c_str();

		if (strcmp(d, "down") == 0)
		{
			down = true;
		}
		else if (strcmp(d, "up") == 0)
		{
			down = false;
		}
		else
		{
			Napi::Error::New(env, "Invalid mouse button state specified.").ThrowAsJavaScriptException();
return env.Null();
		}
	}

	if (info.Length() == 2)
	{
		std::string bstr = info[1].As<Napi::String>();
		const char * const b = bstr.c_str();

		switch (CheckMouseButton(b, &button))
		{
			case -1:
				Napi::Error::New(env, "Null pointer in mouse button code.").ThrowAsJavaScriptException();
return env.Null();
				break;
			case -2:
				Napi::Error::New(env, "Invalid mouse button specified.").ThrowAsJavaScriptException();
return env.Null();
				break;
		}
	}
	else if (info.Length() > 2)
	{
		Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}

	toggleMouse(down, button);
	microsleep(mouseDelay);

	return Napi::Number::New(env, 1);
}

Napi::Value setMouseDelayWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 1)
	{
		Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}

	mouseDelay = info[0].As<Napi::Number>().Int32Value();

	return Napi::Number::New(env, 1);
}

Napi::Value scrollMouseWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 2)
	{
    	Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}

	int x = info[0].As<Napi::Number>().Int32Value();
	int y = info[1].As<Napi::Number>().Int32Value();

	scrollMouse(x, y);
	microsleep(mouseDelay);

	return Napi::Number::New(env, 1);
}
/*
 _  __          _                         _
| |/ /___ _   _| |__   ___   __ _ _ __ __| |
| ' // _ \ | | | '_ \ / _ \ / _` | '__/ _` |
| . \  __/ |_| | |_) | (_) | (_| | | | (_| |
|_|\_\___|\__, |_.__/ \___/ \__,_|_|  \__,_|
          |___/
*/
struct KeyNames
{
	const char* name;
	MMKeyCode   key;
};

static KeyNames key_names[] =
{
	{ "backspace",      K_BACKSPACE },
	{ "delete",         K_DELETE },
	{ "enter",          K_RETURN },
	{ "tab",            K_TAB },
	{ "escape",         K_ESCAPE },
	{ "up",             K_UP },
	{ "down",           K_DOWN },
	{ "right",          K_RIGHT },
	{ "left",           K_LEFT },
	{ "home",           K_HOME },
	{ "end",            K_END },
	{ "pageup",         K_PAGEUP },
	{ "pagedown",       K_PAGEDOWN },
	{ "f1",             K_F1 },
	{ "f2",             K_F2 },
	{ "f3",             K_F3 },
	{ "f4",             K_F4 },
	{ "f5",             K_F5 },
	{ "f6",             K_F6 },
	{ "f7",             K_F7 },
	{ "f8",             K_F8 },
	{ "f9",             K_F9 },
	{ "f10",            K_F10 },
	{ "f11",            K_F11 },
	{ "f12",            K_F12 },
	{ "f13",            K_F13 },
	{ "f14",            K_F14 },
	{ "f15",            K_F15 },
	{ "f16",            K_F16 },
	{ "f17",            K_F17 },
	{ "f18",            K_F18 },
	{ "f19",            K_F19 },
	{ "f20",            K_F20 },
	{ "f21",            K_F21 },
	{ "f22",            K_F22 },
	{ "f23",            K_F23 },
	{ "f24",            K_F24 },
	{ "capslock",       K_CAPSLOCK },
	{ "command",        K_META },
	{ "alt",            K_ALT },
	{ "right_alt",      K_RIGHT_ALT },
	{ "control",        K_CONTROL },
	{ "left_control",   K_LEFT_CONTROL },
	{ "right_control",  K_RIGHT_CONTROL },
	{ "shift",          K_SHIFT },
	{ "right_shift",    K_RIGHTSHIFT },
	{ "space",          K_SPACE },
	{ "printscreen",    K_PRINTSCREEN },
	{ "insert",         K_INSERT },
	{ "menu",           K_MENU },

	{ "audio_mute",     K_AUDIO_VOLUME_MUTE },
	{ "audio_vol_down", K_AUDIO_VOLUME_DOWN },
	{ "audio_vol_up",   K_AUDIO_VOLUME_UP },
	{ "audio_play",     K_AUDIO_PLAY },
	{ "audio_stop",     K_AUDIO_STOP },
	{ "audio_pause",    K_AUDIO_PAUSE },
	{ "audio_prev",     K_AUDIO_PREV },
	{ "audio_next",     K_AUDIO_NEXT },
	{ "audio_rewind",   K_AUDIO_REWIND },
	{ "audio_forward",  K_AUDIO_FORWARD },
	{ "audio_repeat",   K_AUDIO_REPEAT },
	{ "audio_random",   K_AUDIO_RANDOM },

	{ "numpad_lock",	K_NUMPAD_LOCK },
	{ "numpad_0",		K_NUMPAD_0 },
	{ "numpad_0",		K_NUMPAD_0 },
	{ "numpad_1",		K_NUMPAD_1 },
	{ "numpad_2",		K_NUMPAD_2 },
	{ "numpad_3",		K_NUMPAD_3 },
	{ "numpad_4",		K_NUMPAD_4 },
	{ "numpad_5",		K_NUMPAD_5 },
	{ "numpad_6",		K_NUMPAD_6 },
	{ "numpad_7",		K_NUMPAD_7 },
	{ "numpad_8",		K_NUMPAD_8 },
	{ "numpad_9",		K_NUMPAD_9 },
	{ "numpad_+",		K_NUMPAD_PLUS },
	{ "numpad_-",		K_NUMPAD_MINUS },
	{ "numpad_*",		K_NUMPAD_MULTIPLY },
	{ "numpad_/",		K_NUMPAD_DIVIDE },
	{ "numpad_.",		K_NUMPAD_DECIMAL },

	{ "lights_mon_up",    K_LIGHTS_MON_UP },
	{ "lights_mon_down",  K_LIGHTS_MON_DOWN },
	{ "lights_kbd_toggle",K_LIGHTS_KBD_TOGGLE },
	{ "lights_kbd_up",    K_LIGHTS_KBD_UP },
	{ "lights_kbd_down",  K_LIGHTS_KBD_DOWN },

	{ NULL,               K_NOT_A_KEY } /* end marker */
};

int CheckKeyCodes(const char* k, MMKeyCode *key)
{
	if (!key) return -1;

	if (strlen(k) == 1)
	{
		*key = keyCodeForChar(*k);
		return 0;
	}

	*key = K_NOT_A_KEY;

	KeyNames* kn = key_names;
	while (kn->name)
	{
		if (strcmp(k, kn->name) == 0)
		{
			*key = kn->key;
			break;
		}
		kn++;
	}

	if (*key == K_NOT_A_KEY)
	{
		return -2;
	}

	return 0;
}

int CheckKeyFlags(const char* f, MMKeyFlags* flags)
{
	if (!flags) return -1;

	if (strcmp(f, "alt") == 0 || strcmp(f, "right_alt") == 0)
	{
		*flags = MOD_ALT;
	}
	else if(strcmp(f, "command") == 0)
	{
		*flags = MOD_META;
	}
	else if(strcmp(f, "control") == 0 || strcmp(f, "right_control") == 0 || strcmp(f, "left_control") == 0)
	{
		*flags = MOD_CONTROL;
	}
	else if(strcmp(f, "shift") == 0 || strcmp(f, "right_shift") == 0)
	{
		*flags = MOD_SHIFT;
	}
	else if(strcmp(f, "none") == 0)
	{
		*flags = MOD_NONE;
	}
	else
	{
		return -2;
	}

	return 0;
}

int GetFlagsFromString(Napi::Value value, MMKeyFlags* flags) {
	Napi::Env env = value.Env();
	Napi::String fstr(env, value.ToString());
	return CheckKeyFlags(fstr.Utf8Value().c_str(), flags);
}

int GetFlagsFromValue(Napi::Value value, MMKeyFlags* flags) {
	if (!flags) return -1;

	//Optionally allow an array of flag strings to be passed.
	if (value.IsArray())
	{
		Napi::Array a = value.As<Napi::Array>();
		for (uint32_t i = 0; i < a.Length(); i++)
		{
		  if ((a).Has(i)) {
                Napi::Value v((a).Get(i));
                if (!v.IsString()) return -2;

                MMKeyFlags f = MOD_NONE;
                const int rv = GetFlagsFromString(v, &f);
                if (rv) return rv;

                *flags = (MMKeyFlags)(*flags | f);
			}
		}
		return 0;
	}

	//If it's not an array, it should be a single string value.
	return GetFlagsFromString(value, flags);
}

Napi::Value keyTapWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	MMKeyFlags flags = MOD_NONE;
	MMKeyCode key;
	const char *k;

	Napi::String kstr(env, info[0].ToString());
	k = kstr.Utf8Value().c_str();

	switch (info.Length())
	{
		case 2:
			switch (GetFlagsFromValue(info[1], &flags))
			{
				case -1:
					Napi::Error::New(env, "Null pointer in key flag.").ThrowAsJavaScriptException();
return env.Null();
					break;
				case -2:
					Napi::Error::New(env, "Invalid key flag specified.").ThrowAsJavaScriptException();
return env.Null();
					break;
			}
			break;
		case 1:
			break;
		default:
			Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}

	switch(CheckKeyCodes(k, &key))
	{
		case -1:
			Napi::Error::New(env, "Null pointer in key code.").ThrowAsJavaScriptException();
return env.Null();
			break;
		case -2:
			Napi::Error::New(env, "Invalid key code specified.").ThrowAsJavaScriptException();
return env.Null();
			break;
		default:
			toggleKeyCode(key, true, flags);
			microsleep(keyboardDelay);
			toggleKeyCode(key, false, flags);
			microsleep(keyboardDelay);
			break;
	}

	return Napi::Number::New(env, 1);
}


Napi::Value keyToggleWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	MMKeyFlags flags = MOD_NONE;
	MMKeyCode key;

	bool down;
	const char *k;

	//Get arguments from JavaScript.
	std::string kstr = info[0].As<Napi::String>();

	//Convert arguments to chars.
	k = kstr.c_str();

	//Check and confirm number of arguments.
	switch (info.Length())
	{
		case 3:
			//Get key modifier.
			switch (GetFlagsFromValue(info[2], &flags))
			{
				case -1:
					Napi::Error::New(env, "Null pointer in key flag.").ThrowAsJavaScriptException();
return env.Null();
					break;
				case -2:
					Napi::Error::New(env, "Invalid key flag specified.").ThrowAsJavaScriptException();
return env.Null();
					break;
			}
			break;
		case 2:
			break;
		default:
			Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}

	//Get down value if provided.
	if (info.Length() > 1)
	{
		const char *d;

		std::string dstr = info[1].As<Napi::String>();
		d = dstr.c_str();

		if (strcmp(d, "down") == 0)
		{
			down = true;
		}
		else if (strcmp(d, "up") == 0)
		{
			down = false;
		}
		else
		{
			Napi::Error::New(env, "Invalid key state specified.").ThrowAsJavaScriptException();
return env.Null();
		}
	}

	//Get the actual key.
	switch(CheckKeyCodes(k, &key))
	{
		case -1:
			Napi::Error::New(env, "Null pointer in key code.").ThrowAsJavaScriptException();
return env.Null();
			break;
		case -2:
			Napi::Error::New(env, "Invalid key code specified.").ThrowAsJavaScriptException();
return env.Null();
			break;
		default:
			toggleKeyCode(key, down, flags);
			microsleep(keyboardDelay);
	}

	return Napi::Number::New(env, 1);
}

Napi::Value unicodeTapWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if (info.Length() == 0)
	{
		Napi::Error::New(env, "Invalid character typed.").ThrowAsJavaScriptException();
return env.Null();
	}

	size_t value = static_cast<size_t>(info[0].ToNumber().Int32Value());

	if (value != 0) {
		unicodeTap(value);

		return Napi::Number::New(env, 1);
	} else {
		Napi::Error::New(env, "Invalid character typed.").ThrowAsJavaScriptException();
return env.Null();
	}
}

Napi::Value typeStringWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if (info.Length() > 0) {
		const char *s;
		std::string str = info[0].As<Napi::String>();

		s = str.c_str();

		#if defined(IS_MACOSX)
			typeStringDelayed(s, MACOS_TYPE_STRING_DEFAULT_CPM);
		#else
			typeStringDelayed(s, 0);
		#endif

		return Napi::Number::New(env, 1);
	} else {
		Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}
}

Napi::Value typeStringDelayedWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if (info.Length() > 0) {
		const char *s;
		std::string str = info[0].As<Napi::String>();

		s = str.c_str();

	size_t cpm = info[1].As<Napi::Number>().Int32Value();

		typeStringDelayed(s, cpm);

		return Napi::Number::New(env, 1);
	} else {
		Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}
}

Napi::Value setKeyboardDelayWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 1)
	{
		Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}

	keyboardDelay = info[0].As<Napi::Number>().Int32Value();

	return Napi::Number::New(env, 1);
}

/*
  ____
 / ___|  ___ _ __ ___  ___ _ __
 \___ \ / __| '__/ _ \/ _ \ '_ \
  ___) | (__| | |  __/  __/ | | |
 |____/ \___|_|  \___|\___|_| |_|

*/

/**
 * Pad hex color code with leading zeros.
 * @param color Hex value to pad.
 * @param hex   Hex value to output.
 */
void padHex(MMRGBHex color, char* hex)
{
	//Length needs to be 7 because snprintf includes a terminating null.
	//Use %06x to pad hex value with leading 0s.
	snprintf(hex, 7, "%06x", color);
}

static bool multiplySizeT(size_t left, size_t right, size_t *result)
{
	if (left != 0 && right > (SIZE_MAX / left)) {
		return false;
	}

	*result = left * right;
	return true;
}

static bool parseSizeT(Napi::Env env, Napi::Value value, const char *name, size_t *result)
{
	if (!value.IsNumber()) {
		Napi::Error::New(env, std::string(name) + " must be a number.")
			.ThrowAsJavaScriptException();
		return false;
	}

	double number = value.As<Napi::Number>().DoubleValue();
	if (number < 0) {
		Napi::Error::New(env, std::string(name) + " must be non-negative.")
			.ThrowAsJavaScriptException();
		return false;
	}

	*result = static_cast<size_t>(number);
	return true;
}

static bool parseToleranceOption(Napi::Env env, Napi::Object options, float *tolerance)
{
	if (!options.Has("tolerance")) {
		return true;
	}

	if (!options.Get("tolerance").IsNumber()) {
		Napi::Error::New(env, "tolerance must be a number.")
			.ThrowAsJavaScriptException();
		return false;
	}

	double value = options.Get("tolerance").As<Napi::Number>().DoubleValue();
	if (value < 0.0 || value > 1.0) {
		Napi::Error::New(env, "tolerance must be between 0.0 and 1.0.")
			.ThrowAsJavaScriptException();
		return false;
	}

	*tolerance = static_cast<float>(value);
	return true;
}

static MMBitmapRef createBorrowedBitmap(Napi::Env env, Napi::Object obj)
{
	size_t width;
	size_t height;
	size_t byteWidth;
	size_t rowBytes;
	size_t bufferSize;
	size_t parsedBitsPerPixel;
	size_t parsedBytesPerPixel;
	uint8_t bitsPerPixel;
	uint8_t bytesPerPixel;

	if (!obj.Has("width") || !obj.Has("height") || !obj.Has("byteWidth") ||
	    !obj.Has("bitsPerPixel") || !obj.Has("bytesPerPixel") || !obj.Has("image")) {
		Napi::Error::New(env, "Bitmap object is missing required properties.")
			.ThrowAsJavaScriptException();
		return NULL;
	}

	if (!parseSizeT(env, obj.Get("width"), "width", &width) ||
	    !parseSizeT(env, obj.Get("height"), "height", &height) ||
	    !parseSizeT(env, obj.Get("byteWidth"), "byteWidth", &byteWidth) ||
	    !parseSizeT(env, obj.Get("bitsPerPixel"), "bitsPerPixel", &parsedBitsPerPixel) ||
	    !parseSizeT(env, obj.Get("bytesPerPixel"), "bytesPerPixel", &parsedBytesPerPixel)) {
		return NULL;
	}

	bitsPerPixel = static_cast<uint8_t>(parsedBitsPerPixel);
	bytesPerPixel = static_cast<uint8_t>(parsedBytesPerPixel);

	if (!obj.Get("image").IsBuffer()) {
		Napi::Error::New(env, "image must be a Buffer.").ThrowAsJavaScriptException();
		return NULL;
	}

	if (width == 0 || height == 0) {
		Napi::Error::New(env, "Bitmap width and height must be greater than zero.")
			.ThrowAsJavaScriptException();
		return NULL;
	}

	if ((bytesPerPixel != 3 && bytesPerPixel != 4) ||
	    bitsPerPixel != (bytesPerPixel * 8)) {
		Napi::Error::New(env, "Bitmap must use 24-bit or 32-bit pixels.")
			.ThrowAsJavaScriptException();
		return NULL;
	}

	if (!multiplySizeT(width, bytesPerPixel, &rowBytes) || byteWidth < rowBytes) {
		Napi::Error::New(env, "byteWidth is smaller than the bitmap row size.")
			.ThrowAsJavaScriptException();
		return NULL;
	}

	if (!multiplySizeT(byteWidth, height, &bufferSize)) {
		Napi::Error::New(env, "Bitmap buffer size is too large.")
			.ThrowAsJavaScriptException();
		return NULL;
	}

	Napi::Buffer<uint8_t> image = obj.Get("image").As<Napi::Buffer<uint8_t> >();
	if (image.Length() < bufferSize) {
		Napi::Error::New(env, "Bitmap image buffer is smaller than byteWidth * height.")
			.ThrowAsJavaScriptException();
		return NULL;
	}

	MMBitmapRef bitmap = createMMBitmapWithCleanup(image.Data(),
	                                               width,
	                                               height,
	                                               byteWidth,
	                                               bitsPerPixel,
	                                               bytesPerPixel,
	                                               NULL,
	                                               NULL);
	if (bitmap == NULL) {
		Napi::Error::New(env, "Failed to create bitmap.")
			.ThrowAsJavaScriptException();
	}

	return bitmap;
}

static bool parseSearchRect(Napi::Env env,
                            Napi::Value optionsValue,
                            MMBitmapRef bitmap,
                            MMRect *rect,
                            float *tolerance)
{
	size_t x = 0;
	size_t y = 0;
	size_t width = bitmap->width;
	size_t height = bitmap->height;
	bool hasWidth = false;
	bool hasHeight = false;

	*tolerance = 0.0f;

	if (optionsValue.IsUndefined() || optionsValue.IsNull()) {
		*rect = MMBitmapGetBounds(bitmap);
		return true;
	}

	if (!optionsValue.IsObject()) {
		Napi::Error::New(env, "Search options must be an object.")
			.ThrowAsJavaScriptException();
		return false;
	}

	Napi::Object options = optionsValue.As<Napi::Object>();
	if (options.Has("x") && !parseSizeT(env, options.Get("x"), "x", &x)) {
		return false;
	}
	if (options.Has("y") && !parseSizeT(env, options.Get("y"), "y", &y)) {
		return false;
	}
	if (options.Has("width")) {
		hasWidth = true;
		if (!parseSizeT(env, options.Get("width"), "width", &width)) {
			return false;
		}
	}
	if (options.Has("height")) {
		hasHeight = true;
		if (!parseSizeT(env, options.Get("height"), "height", &height)) {
			return false;
		}
	}
	if (!parseToleranceOption(env, options, tolerance)) {
		return false;
	}

	if (x > bitmap->width || y > bitmap->height) {
		Napi::Error::New(env, "Search origin is outside the bitmap bounds.")
			.ThrowAsJavaScriptException();
		return false;
	}

	if (!hasWidth) {
		width = bitmap->width - x;
	}
	if (!hasHeight) {
		height = bitmap->height - y;
	}

	*rect = MMRectMake(x, y, width, height);
	if (width == 0 || height == 0 || !MMBitmapRectInBounds(bitmap, *rect)) {
		Napi::Error::New(env, "Search rect must stay within the bitmap bounds.")
			.ThrowAsJavaScriptException();
		return false;
	}

	return true;
}

static bool parseHexColor(Napi::Env env, Napi::Value value, MMRGBHex *color)
{
	std::string hex;
	size_t i;

	if (!value.IsString()) {
		Napi::Error::New(env, "Color must be a 6-digit hex string.")
			.ThrowAsJavaScriptException();
		return false;
	}

	hex = value.As<Napi::String>().Utf8Value();
	if (!hex.empty() && hex[0] == '#') {
		hex.erase(0, 1);
	}

	if (hex.length() != 6) {
		Napi::Error::New(env, "Color must be a 6-digit hex string.")
			.ThrowAsJavaScriptException();
		return false;
	}

	for (i = 0; i < hex.length(); ++i) {
		if (!isxdigit((unsigned char)hex[i])) {
			Napi::Error::New(env, "Color must be a 6-digit hex string.")
				.ThrowAsJavaScriptException();
			return false;
		}
	}

	*color = static_cast<MMRGBHex>(strtoul(hex.c_str(), NULL, 16));
	return true;
}

static Napi::Object createPointObject(Napi::Env env, MMPoint point)
{
	Napi::Object obj = Napi::Object::New(env);
	obj.Set("x", Napi::Number::New(env, point.x));
	obj.Set("y", Napi::Number::New(env, point.y));
	return obj;
}

static Napi::Array createPointArray(Napi::Env env, MMPointArrayRef points)
{
	Napi::Array array = Napi::Array::New(env, points->count);
	size_t i;

	for (i = 0; i < points->count; ++i) {
		array.Set(i, createPointObject(env, MMPointArrayGetItem(points, i)));
	}

	return array;
}

static void finalizeBitmap(Napi::Env env, uint8_t *data, MMBitmap *bitmap)
{
	if (bitmap != NULL) {
		destroyMMBitmap(bitmap);
	}
}

static Napi::Object createBitmapObject(Napi::Env env, MMBitmapRef bitmap)
{
	size_t bufferSize;

	if (!multiplySizeT(bitmap->bytewidth, bitmap->height, &bufferSize)) {
		destroyMMBitmap(bitmap);
		Napi::Error::New(env, "Bitmap is too large.")
			.ThrowAsJavaScriptException();
		return Napi::Object::New(env);
	}

	Napi::Buffer<uint8_t> buffer = Napi::Buffer<uint8_t>::New(env,
	                                                         bitmap->imageBuffer,
	                                                         bufferSize,
	                                                         finalizeBitmap,
	                                                         bitmap);
	Napi::Object obj = Napi::Object::New(env);
	obj.Set("width", Napi::Number::New(env, bitmap->width));
	obj.Set("height", Napi::Number::New(env, bitmap->height));
	obj.Set("byteWidth", Napi::Number::New(env, bitmap->bytewidth));
	obj.Set("bitsPerPixel", Napi::Number::New(env, bitmap->bitsPerPixel));
	obj.Set("bytesPerPixel", Napi::Number::New(env, bitmap->bytesPerPixel));
	obj.Set("image", buffer);
	return obj;
}

static bool parseImageTypeFromPath(Napi::Env env,
                                   const std::string& path,
                                   MMImageType *imageType)
{
	std::string::size_type dotOffset = path.find_last_of('.');

	if (dotOffset == std::string::npos || dotOffset == path.length() - 1) {
		Napi::Error::New(env, "Unsupported image type. Use a .png or .bmp file.")
			.ThrowAsJavaScriptException();
		return false;
	}

	*imageType = imageTypeFromExtension(path.c_str() + dotOffset + 1);
	if (*imageType == kInvalidImageType) {
		Napi::Error::New(env, "Unsupported image type. Use a .png or .bmp file.")
			.ThrowAsJavaScriptException();
		return false;
	}

	return true;
}

Napi::Value getPixelColorWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if (info.Length() != 2)
	{
		Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
return env.Null();
	}

	MMBitmapRef bitmap;
	MMRGBHex color;

	double x = info[0].ToNumber().DoubleValue();
	double y = info[1].ToNumber().DoubleValue();

	if (x < 0 || y < 0 ||
	    !pointVisibleOnMainDisplay(MMPointMake(static_cast<size_t>(x), static_cast<size_t>(y))))
	{
		Napi::Error::New(env, "Requested coordinates are outside the main screen's dimensions.").ThrowAsJavaScriptException();
return env.Null();
	}

	bitmap = copyMMBitmapFromDisplayInRect(MMRectMake(static_cast<size_t>(x), static_cast<size_t>(y), 1, 1));
	if (bitmap == NULL) {
		Napi::Error::New(env, "Failed to capture the requested pixel.")
			.ThrowAsJavaScriptException();
		return env.Null();
	}

	color = MMRGBHexAtPoint(bitmap, 0, 0);

	char hex[7];

	padHex(color, hex);

	destroyMMBitmap(bitmap);

	return Napi::String::New(env, hex);
}

Napi::Value getScreenSizeWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	//Get display size.
	MMSize displaySize = getMainDisplaySize();

	//Create our return object.
	Napi::Object obj = Napi::Object::New(env);
	obj.Set(Napi::String::New(env, "width"), Napi::Number::New(env, displaySize.width));
	obj.Set(Napi::String::New(env, "height"), Napi::Number::New(env, displaySize.height));

	//Return our object with .width and .height.
	return obj;
}

Napi::Value getXDisplayNameWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	#if defined(USE_X11)
	const char* display = getXDisplay();
	return Napi::String::New(env, display);
	#else
	Napi::Error::New(env, "getXDisplayName is only supported on Linux").ThrowAsJavaScriptException();
	return env.Null();
	#endif
}

Napi::Value setXDisplayNameWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	#if defined(USE_X11)
	std::string string = info[0].As<Napi::String>();
	setXDisplay(string.c_str());
	return Napi::Number::New(env, 1);
	#else
	Napi::Error::New(env, "setXDisplayName is only supported on Linux").ThrowAsJavaScriptException();
	return env.Null();
	#endif
}

Napi::Value captureScreenWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	size_t x = 0;
	size_t y = 0;
	size_t w;
	size_t h;
	MMSize displaySize = getMainDisplaySize();

	//If user has provided screen coords, use them!
	if (info.Length() == 4)
	{
		if (!parseSizeT(env, info[0], "x", &x) ||
		    !parseSizeT(env, info[1], "y", &y) ||
		    !parseSizeT(env, info[2], "width", &w) ||
		    !parseSizeT(env, info[3], "height", &h)) {
			return env.Null();
		}
	}
	else if (info.Length() == 0)
	{
		//We're getting the full screen.
		w = displaySize.width;
		h = displaySize.height;
	}
	else
	{
		Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
		return env.Null();
	}

	if (w == 0 || h == 0 ||
	    x > displaySize.width || y > displaySize.height ||
	    w > displaySize.width - x || h > displaySize.height - y) {
		Napi::Error::New(env, "Requested capture rect is outside the main screen's dimensions.")
			.ThrowAsJavaScriptException();
		return env.Null();
	}

	MMBitmapRef bitmap = copyMMBitmapFromDisplayInRect(MMRectMake(x, y, w, h));
	if (bitmap == NULL) {
		Napi::Error::New(env, "Failed to capture the requested screen rect.")
			.ThrowAsJavaScriptException();
		return env.Null();
	}

	Napi::Object obj = createBitmapObject(env, bitmap);
	obj.Set("screenX", Napi::Number::New(env, x));
	obj.Set("screenY", Napi::Number::New(env, y));
	obj.Set("scaleX", Napi::Number::New(env, (double)bitmap->width / (double)w));
	obj.Set("scaleY", Napi::Number::New(env, (double)bitmap->height / (double)h));
	return obj;
}

/*
 ____  _ _
| __ )(_) |_ _ __ ___   __ _ _ __
|  _ \| | __| '_ ` _ \ / _` | '_ \
| |_) | | |_| | | | | | (_| | |_) |
|____/|_|\__|_| |_| |_|\__,_| .__/
                            |_|
 */

Napi::Value getColorWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	MMRGBHex color;
	size_t x;
	size_t y;

	if (info.Length() != 3)
	{
		Napi::Error::New(env, "Invalid number of arguments.").ThrowAsJavaScriptException();
		return env.Null();
	}

	if (!info[0].IsObject() ||
	    !parseSizeT(env, info[1], "x", &x) ||
	    !parseSizeT(env, info[2], "y", &y)) {
		if (!info[0].IsObject()) {
			Napi::Error::New(env, "First argument must be a bitmap object.")
				.ThrowAsJavaScriptException();
		}
		return env.Null();
	}

	MMBitmapRef bitmap = createBorrowedBitmap(env, info[0].As<Napi::Object>());
	if (bitmap == NULL) {
		return env.Null();
	}

	// Make sure the requested pixel is inside the bitmap.
	if (!MMBitmapPointInBounds(bitmap, MMPointMake(x, y)))
	{
		Napi::Error::New(env, "Requested coordinates are outside the bitmap's dimensions.").ThrowAsJavaScriptException();
		destroyMMBitmap(bitmap);
		return env.Null();
	}

	color = MMRGBHexAtPoint(bitmap, x, y);

	char hex[7];

	padHex(color, hex);

	destroyMMBitmap(bitmap);

	return Napi::String::New(env, hex);

}

Napi::Value findColorWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	MMBitmapRef bitmap;
	MMRGBHex color;
	MMRect rect;
	MMPoint point;
	float tolerance;

	if (info.Length() < 2 || info.Length() > 3 || !info[0].IsObject()) {
		Napi::Error::New(env, "Invalid arguments.").ThrowAsJavaScriptException();
		return env.Null();
	}

	bitmap = createBorrowedBitmap(env, info[0].As<Napi::Object>());
	if (bitmap == NULL) {
		return env.Null();
	}

	if (!parseHexColor(env, info[1], &color) ||
	    !parseSearchRect(env,
	                     info.Length() == 3 ? info[2] : env.Undefined(),
	                     bitmap,
	                     &rect,
	                     &tolerance)) {
		destroyMMBitmap(bitmap);
		return env.Null();
	}

	if (findColorInRect(bitmap, color, &point, rect, tolerance) != 0) {
		destroyMMBitmap(bitmap);
		return env.Null();
	}

	destroyMMBitmap(bitmap);
	return createPointObject(env, point);
}

Napi::Value findColorsWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	MMBitmapRef bitmap;
	MMRGBHex color;
	MMRect rect;
	float tolerance;
	MMPointArrayRef points;
	Napi::Array result;

	if (info.Length() < 2 || info.Length() > 3 || !info[0].IsObject()) {
		Napi::Error::New(env, "Invalid arguments.").ThrowAsJavaScriptException();
		return env.Null();
	}

	bitmap = createBorrowedBitmap(env, info[0].As<Napi::Object>());
	if (bitmap == NULL) {
		return env.Null();
	}

	if (!parseHexColor(env, info[1], &color) ||
	    !parseSearchRect(env,
	                     info.Length() == 3 ? info[2] : env.Undefined(),
	                     bitmap,
	                     &rect,
	                     &tolerance)) {
		destroyMMBitmap(bitmap);
		return env.Null();
	}

	points = findAllColorInRect(bitmap, color, rect, tolerance);
	if (points == NULL) {
		destroyMMBitmap(bitmap);
		Napi::Error::New(env, "Failed to search bitmap colors.")
			.ThrowAsJavaScriptException();
		return env.Null();
	}

	result = createPointArray(env, points);
	destroyMMPointArray(points);
	destroyMMBitmap(bitmap);
	return result;
}

Napi::Value countColorWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	MMBitmapRef bitmap;
	MMRGBHex color;
	MMRect rect;
	float tolerance;
	size_t count;

	if (info.Length() < 2 || info.Length() > 3 || !info[0].IsObject()) {
		Napi::Error::New(env, "Invalid arguments.").ThrowAsJavaScriptException();
		return env.Null();
	}

	bitmap = createBorrowedBitmap(env, info[0].As<Napi::Object>());
	if (bitmap == NULL) {
		return env.Null();
	}

	if (!parseHexColor(env, info[1], &color) ||
	    !parseSearchRect(env,
	                     info.Length() == 3 ? info[2] : env.Undefined(),
	                     bitmap,
	                     &rect,
	                     &tolerance)) {
		destroyMMBitmap(bitmap);
		return env.Null();
	}

	count = countOfColorsInRect(bitmap, color, rect, tolerance);
	destroyMMBitmap(bitmap);
	return Napi::Number::New(env, count);
}

Napi::Value findBitmapWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	MMBitmapRef haystack;
	MMBitmapRef needle;
	MMRect rect;
	MMPoint point;
	float tolerance;

	if (info.Length() < 2 || info.Length() > 3 ||
	    !info[0].IsObject() || !info[1].IsObject()) {
		Napi::Error::New(env, "Invalid arguments.").ThrowAsJavaScriptException();
		return env.Null();
	}

	haystack = createBorrowedBitmap(env, info[0].As<Napi::Object>());
	if (haystack == NULL) {
		return env.Null();
	}

	needle = createBorrowedBitmap(env, info[1].As<Napi::Object>());
	if (needle == NULL) {
		destroyMMBitmap(haystack);
		return env.Null();
	}

	if (!parseSearchRect(env,
	                     info.Length() == 3 ? info[2] : env.Undefined(),
	                     haystack,
	                     &rect,
	                     &tolerance)) {
		destroyMMBitmap(needle);
		destroyMMBitmap(haystack);
		return env.Null();
	}

	if (findBitmapInRect(needle, haystack, &point, rect, tolerance) != 0) {
		destroyMMBitmap(needle);
		destroyMMBitmap(haystack);
		return env.Null();
	}

	destroyMMBitmap(needle);
	destroyMMBitmap(haystack);
	return createPointObject(env, point);
}

Napi::Value findBitmapsWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	MMBitmapRef haystack;
	MMBitmapRef needle;
	MMRect rect;
	float tolerance;
	MMPointArrayRef points;
	Napi::Array result;

	if (info.Length() < 2 || info.Length() > 3 ||
	    !info[0].IsObject() || !info[1].IsObject()) {
		Napi::Error::New(env, "Invalid arguments.").ThrowAsJavaScriptException();
		return env.Null();
	}

	haystack = createBorrowedBitmap(env, info[0].As<Napi::Object>());
	if (haystack == NULL) {
		return env.Null();
	}

	needle = createBorrowedBitmap(env, info[1].As<Napi::Object>());
	if (needle == NULL) {
		destroyMMBitmap(haystack);
		return env.Null();
	}

	if (!parseSearchRect(env,
	                     info.Length() == 3 ? info[2] : env.Undefined(),
	                     haystack,
	                     &rect,
	                     &tolerance)) {
		destroyMMBitmap(needle);
		destroyMMBitmap(haystack);
		return env.Null();
	}

	points = findAllBitmapInRect(needle, haystack, rect, tolerance);
	if (points == NULL) {
		destroyMMBitmap(needle);
		destroyMMBitmap(haystack);
		Napi::Error::New(env, "Failed to search for bitmaps.")
			.ThrowAsJavaScriptException();
		return env.Null();
	}

	result = createPointArray(env, points);
	destroyMMPointArray(points);
	destroyMMBitmap(needle);
	destroyMMBitmap(haystack);
	return result;
}

Napi::Value countBitmapWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	MMBitmapRef haystack;
	MMBitmapRef needle;
	MMRect rect;
	float tolerance;
	size_t count;

	if (info.Length() < 2 || info.Length() > 3 ||
	    !info[0].IsObject() || !info[1].IsObject()) {
		Napi::Error::New(env, "Invalid arguments.").ThrowAsJavaScriptException();
		return env.Null();
	}

	haystack = createBorrowedBitmap(env, info[0].As<Napi::Object>());
	if (haystack == NULL) {
		return env.Null();
	}

	needle = createBorrowedBitmap(env, info[1].As<Napi::Object>());
	if (needle == NULL) {
		destroyMMBitmap(haystack);
		return env.Null();
	}

	if (!parseSearchRect(env,
	                     info.Length() == 3 ? info[2] : env.Undefined(),
	                     haystack,
	                     &rect,
	                     &tolerance)) {
		destroyMMBitmap(needle);
		destroyMMBitmap(haystack);
		return env.Null();
	}

	count = countOfBitmapInRect(needle, haystack, rect, tolerance);
	destroyMMBitmap(needle);
	destroyMMBitmap(haystack);
	return Napi::Number::New(env, count);
}

Napi::Value loadImageWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	std::string path;
	MMImageType imageType;
	MMIOError error = kMMIOUnsupportedTypeError;
	MMBitmapRef bitmap;

	if (info.Length() != 1 || !info[0].IsString()) {
		Napi::Error::New(env, "loadImage expects a single file path string.")
			.ThrowAsJavaScriptException();
		return env.Null();
	}

	path = info[0].As<Napi::String>().Utf8Value();
	if (!parseImageTypeFromPath(env, path, &imageType)) {
		return env.Null();
	}

	bitmap = newMMBitmapFromFile(path.c_str(), imageType, &error);
	if (bitmap == NULL) {
		const char *message = MMIOErrorString(imageType, error);
		if (message == NULL) {
			message = "Failed to load image.";
		}
		Napi::Error::New(env, message).ThrowAsJavaScriptException();
		return env.Null();
	}

	return createBitmapObject(env, bitmap);
}

Napi::Value saveImageWrapper(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	std::string path;
	MMImageType imageType;
	MMBitmapRef bitmap;

	if (info.Length() != 2 || !info[0].IsObject() || !info[1].IsString()) {
		Napi::Error::New(env, "saveImage expects a bitmap and a file path string.")
			.ThrowAsJavaScriptException();
		return env.Null();
	}

	bitmap = createBorrowedBitmap(env, info[0].As<Napi::Object>());
	if (bitmap == NULL) {
		return env.Null();
	}

	path = info[1].As<Napi::String>().Utf8Value();
	if (!parseImageTypeFromPath(env, path, &imageType)) {
		destroyMMBitmap(bitmap);
		return env.Null();
	}

#if !ROBOTJS_HAS_PNG
	if (imageType == kPNGImageType) {
		destroyMMBitmap(bitmap);
		Napi::Error::New(env, "PNG support is not enabled in this build.")
			.ThrowAsJavaScriptException();
		return env.Null();
	}
#endif

	if (saveMMBitmapToFile(bitmap, path.c_str(), imageType) != 0) {
		destroyMMBitmap(bitmap);
		Napi::Error::New(env, "Failed to save image.")
			.ThrowAsJavaScriptException();
			return env.Null();
	}

	destroyMMBitmap(bitmap);
	return Napi::Boolean::New(env, true);
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{
	exports.Set(Napi::String::New(env, "dragMouse"),
				Napi::Function::New(env, dragMouseWrapper));

	exports.Set(Napi::String::New(env, "updateScreenMetrics"),
				Napi::Function::New(env, updateScreenMetricsWrapper));

	exports.Set(Napi::String::New(env, "moveMouse"),
				Napi::Function::New(env, moveMouseWrapper));

	exports.Set(Napi::String::New(env, "moveMouseSmooth"),
				Napi::Function::New(env, moveMouseSmoothWrapper));

	exports.Set(Napi::String::New(env, "getMousePos"),
				Napi::Function::New(env, getMousePosWrapper));

	exports.Set(Napi::String::New(env, "mouseClick"),
				Napi::Function::New(env, mouseClickWrapper));

	exports.Set(Napi::String::New(env, "mouseToggle"),
				Napi::Function::New(env, mouseToggleWrapper));

	exports.Set(Napi::String::New(env, "scrollMouse"),
				Napi::Function::New(env, scrollMouseWrapper));

	exports.Set(Napi::String::New(env, "setMouseDelay"),
				Napi::Function::New(env, setMouseDelayWrapper));

	exports.Set(Napi::String::New(env, "keyTap"),
				Napi::Function::New(env, keyTapWrapper));

	exports.Set(Napi::String::New(env, "keyToggle"),
				Napi::Function::New(env, keyToggleWrapper));

	exports.Set(Napi::String::New(env, "unicodeTap"),
				Napi::Function::New(env, unicodeTapWrapper));

	exports.Set(Napi::String::New(env, "typeString"),
				Napi::Function::New(env, typeStringWrapper));

	exports.Set(Napi::String::New(env, "typeStringDelayed"),
				Napi::Function::New(env, typeStringDelayedWrapper));

	exports.Set(Napi::String::New(env, "setKeyboardDelay"),
				Napi::Function::New(env, setKeyboardDelayWrapper));

	exports.Set(Napi::String::New(env, "getPixelColor"),
				Napi::Function::New(env, getPixelColorWrapper));

	exports.Set(Napi::String::New(env, "getScreenSize"),
				Napi::Function::New(env, getScreenSizeWrapper));

	exports.Set(Napi::String::New(env, "captureScreen"),
				Napi::Function::New(env, captureScreenWrapper));

	exports.Set(Napi::String::New(env, "getColor"),
				Napi::Function::New(env, getColorWrapper));

	exports.Set(Napi::String::New(env, "findColor"),
				Napi::Function::New(env, findColorWrapper));

	exports.Set(Napi::String::New(env, "findColors"),
				Napi::Function::New(env, findColorsWrapper));

	exports.Set(Napi::String::New(env, "countColor"),
				Napi::Function::New(env, countColorWrapper));

	exports.Set(Napi::String::New(env, "findImage"),
				Napi::Function::New(env, findBitmapWrapper));

	exports.Set(Napi::String::New(env, "findImages"),
				Napi::Function::New(env, findBitmapsWrapper));

	exports.Set(Napi::String::New(env, "countImage"),
				Napi::Function::New(env, countBitmapWrapper));

	exports.Set(Napi::String::New(env, "loadImage"),
				Napi::Function::New(env, loadImageWrapper));

	exports.Set(Napi::String::New(env, "saveImage"),
				Napi::Function::New(env, saveImageWrapper));

	exports.Set(Napi::String::New(env, "hasPNGSupport"),
				Napi::Boolean::New(env, ROBOTJS_HAS_PNG != 0));

	exports.Set(Napi::String::New(env, "getXDisplayName"),
				Napi::Function::New(env, getXDisplayNameWrapper));

	exports.Set(Napi::String::New(env, "setXDisplayName"),
				Napi::Function::New(env, setXDisplayNameWrapper));

	return exports;
}

NODE_API_MODULE(robotjs, InitAll)
