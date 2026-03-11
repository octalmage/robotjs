#include <node_api.h>
#include <vector>
#include <string.h>
#include "mouse.h"
#include "deadbeef_rand.h"
#include "keypress.h"
#include "screen.h"
#include "screengrab.h"
#include "MMBitmap.h"
#include "snprintf.h"
#include "microsleep.h"
#if defined(USE_X11)
	#include "xdisplay.h"
#endif

// Global delays.
int mouseDelay = 10;
int keyboardDelay = 10;

/*
 __  __
|  \/  | ___  _   _ ___  ___
| |\/| |/ _ \| | | / __|/ _ \
| |  | | (_) | |_| \__ \  __/
|_|  |_|\___/ \__,_|___/\___|

*/
int CheckMouseButton(const char * const b, MMMouseButton * const button) {
	if (!button) return -1;
	if (strcmp(b, "left") == 0) {
		*button = LEFT_BUTTON;
	} else if (strcmp(b, "right") == 0) {
		*button = RIGHT_BUTTON;
	} else if (strcmp(b, "middle") == 0) {
		*button = CENTER_BUTTON;
	} else {
		return -2;
	}
	return 0;
}

napi_value DragMouse(napi_env env, napi_callback_info info) {
	size_t argc = 3;
	napi_value args[3];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc < 2 || argc > 3) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	int32_t x, y;
	napi_get_value_int32(env, args[0], &x);
	napi_get_value_int32(env, args[1], &y);
	MMMouseButton button = LEFT_BUTTON;

	if (argc == 3) {
		size_t str_size;
		napi_get_value_string_utf8(env, args[2], NULL, 0, &str_size);
		char* b = (char*)malloc(str_size + 1);
		napi_get_value_string_utf8(env, args[2], b, str_size + 1, &str_size);

		switch (CheckMouseButton(b, &button)) {
			case -1:
				napi_throw_error(env, NULL, "Null pointer in mouse button code.");
				free(b);
				return NULL;
			case -2:
				napi_throw_error(env, NULL, "Invalid mouse button specified.");
				free(b);
				return NULL;
		}
		free(b);
	}

	MMSignedPoint point = MMSignedPointMake(x, y);
	dragMouse(point, button);
	microsleep(mouseDelay);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
}

napi_value UpdateScreenMetrics(napi_env env, napi_callback_info info) {
	updateScreenMetrics();

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
}

napi_value MoveMouse(napi_env env, napi_callback_info info) {
	size_t argc = 2;
	napi_value args[2];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc != 2) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	int32_t x, y;
	napi_get_value_int32(env, args[0], &x);
	napi_get_value_int32(env, args[1], &y);

	MMSignedPoint point = MMSignedPointMake(x, y);
	moveMouse(point);
	microsleep(mouseDelay);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
}

napi_value MoveMouseSmooth(napi_env env, napi_callback_info info) {
	size_t argc = 3;
	napi_value args[3];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc < 2 || argc > 3) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	int32_t x, y;
	napi_get_value_int32(env, args[0], &x);
	napi_get_value_int32(env, args[1], &y);

	MMPoint point = MMPointMake(x, y);
	if (argc == 3) {
		int32_t speed;
		napi_get_value_int32(env, args[2], &speed);
		smoothlyMoveMouse(point, speed);
	} else {
		smoothlyMoveMouse(point, 3.0);
	}
	microsleep(mouseDelay);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
}

napi_value GetMousePos(napi_env env, napi_callback_info info) {
	MMPoint pos = getMousePos();

	napi_value obj;
	napi_create_object(env, &obj);
	napi_value x, y;
	napi_create_int32(env, (int)pos.x, &x);
	napi_create_int32(env, (int)pos.y, &y);
	napi_set_named_property(env, obj, "x", x);
	napi_set_named_property(env, obj, "y", y);

	return obj;
}

napi_value MouseClick(napi_env env, napi_callback_info info) {
	size_t argc = 2;
	napi_value args[2];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	MMMouseButton button = LEFT_BUTTON;
	bool doubleC = false;

	if (argc > 0) {
		size_t str_size;
		napi_get_value_string_utf8(env, args[0], NULL, 0, &str_size);
		char* b = (char*)malloc(str_size + 1);
		napi_get_value_string_utf8(env, args[0], b, str_size + 1, &str_size);

		switch (CheckMouseButton(b, &button)) {
			case -1:
				napi_throw_error(env, NULL, "Null pointer in mouse button code.");
				free(b);
				return NULL;
			case -2:
				napi_throw_error(env, NULL, "Invalid mouse button specified.");
				free(b);
				return NULL;
		}
		free(b);
	}

	if (argc == 2) {
		napi_get_value_bool(env, args[1], &doubleC);
	} else if (argc > 2) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	if (!doubleC) {
		clickMouse(button);
	} else {
		doubleClick(button);
	}
	microsleep(mouseDelay);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
}

napi_value MouseToggle(napi_env env, napi_callback_info info) {
	size_t argc = 2;
	napi_value args[2];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	MMMouseButton button = LEFT_BUTTON;
	bool down = false;

	if (argc > 0) {
		size_t str_size;
		napi_get_value_string_utf8(env, args[0], NULL, 0, &str_size);
		char* d = (char*)malloc(str_size + 1);
		napi_get_value_string_utf8(env, args[0], d, str_size + 1, &str_size);

		if (strcmp(d, "down") == 0) {
			down = true;
		} else if (strcmp(d, "up") == 0) {
			down = false;
		} else {
			napi_throw_error(env, NULL, "Invalid mouse button state specified.");
			free(d);
			return NULL;
		}
		free(d);
	}

	if (argc == 2) {
		size_t str_size;
		napi_get_value_string_utf8(env, args[1], NULL, 0, &str_size);
		char* b = (char*)malloc(str_size + 1);
		napi_get_value_string_utf8(env, args[1], b, str_size + 1, &str_size);

		switch (CheckMouseButton(b, &button)) {
			case -1:
				napi_throw_error(env, NULL, "Null pointer in mouse button code.");
				free(b);
				return NULL;
			case -2:
				napi_throw_error(env, NULL, "Invalid mouse button specified.");
				free(b);
				return NULL;
		}
		free(b);
	} else if (argc > 2) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	toggleMouse(down, button);
	microsleep(mouseDelay);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
}

napi_value SetMouseDelay(napi_env env, napi_callback_info info) {
	size_t argc = 1;
	napi_value args[1];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc != 1) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	napi_get_value_int32(env, args[0], &mouseDelay);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
}

napi_value ScrollMouse(napi_env env, napi_callback_info info) {
	size_t argc = 2;
	napi_value args[2];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc != 2) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	int32_t x, y;
	napi_get_value_int32(env, args[0], &x);
	napi_get_value_int32(env, args[1], &y);

	scrollMouse(x, y);
	microsleep(mouseDelay);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
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
	else if(strcmp(f, "command") == 0 || strcmp(f, "meta") == 0)
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

int GetFlagsFromString(napi_env env, napi_value value, MMKeyFlags* flags)
{
	size_t str_size;
	napi_get_value_string_utf8(env, value, NULL, 0, &str_size);
	char* fstr = (char*)malloc(str_size + 1);
	napi_get_value_string_utf8(env, value, fstr, str_size + 1, &str_size);
	int result = CheckKeyFlags(fstr, flags);
	free(fstr);
	return result;
}

int GetFlagsFromValue(napi_env env, napi_value value, MMKeyFlags* flags)
{
	if (!flags) return -1;

	bool isArray;
	napi_is_array(env, value, &isArray);

	if (isArray)
	{
		uint32_t length;
		napi_get_array_length(env, value, &length);
		for (uint32_t i = 0; i < length; i++)
		{
			napi_value v;
			napi_get_element(env, value, i, &v);

			bool isString;
			napi_valuetype type;
			napi_typeof(env, v, &type);
			isString = (type == napi_string);
			if (!isString) return -2;

			MMKeyFlags f = MOD_NONE;
			const int rv = GetFlagsFromString(env, v, &f);
			if (rv) return rv;

			*flags = (MMKeyFlags)(*flags | f);
		}
		return 0;
	}

	return GetFlagsFromString(env, value, flags);
}

napi_value KeyTap(napi_env env, napi_callback_info info)
{
	size_t argc = 2;
	napi_value args[2];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc < 1 || argc > 2) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	MMKeyFlags flags = MOD_NONE;
	MMKeyCode key;

	size_t str_size;
	napi_get_value_string_utf8(env, args[0], NULL, 0, &str_size);
	char* k = (char*)malloc(str_size + 1);
	napi_get_value_string_utf8(env, args[0], k, str_size + 1, &str_size);

	if (argc == 2) {
		switch (GetFlagsFromValue(env, args[1], &flags))
		{
			case -1:
				free(k);
				napi_throw_error(env, NULL, "Null pointer in key flag.");
				return NULL;
			case -2:
				free(k);
				napi_throw_error(env, NULL, "Invalid key flag specified.");
				return NULL;
		}
	}

	switch(CheckKeyCodes(k, &key))
	{
		case -1:
			free(k);
			napi_throw_error(env, NULL, "Null pointer in key code.");
			return NULL;
		case -2:
			free(k);
			napi_throw_error(env, NULL, "Invalid key code specified.");
			return NULL;
		default:
			toggleKeyCode(key, true, flags);
			microsleep(keyboardDelay);
			toggleKeyCode(key, false, flags);
			microsleep(keyboardDelay);
			break;
	}

	free(k);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
}

napi_value KeyToggle(napi_env env, napi_callback_info info)
{
	size_t argc = 3;
	napi_value args[3];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc < 1 || argc > 3) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	MMKeyFlags flags = MOD_NONE;
	MMKeyCode key;

	size_t str_size;
	napi_get_value_string_utf8(env, args[0], NULL, 0, &str_size);
	char* k = (char*)malloc(str_size + 1);
	napi_get_value_string_utf8(env, args[0], k, str_size + 1, &str_size);

	bool down = false;
	if (argc > 1) {
		napi_get_value_string_utf8(env, args[1], NULL, 0, &str_size);
		char* d = (char*)malloc(str_size + 1);
		napi_get_value_string_utf8(env, args[1], d, str_size + 1, &str_size);

		if (strcmp(d, "down") == 0) {
			down = true;
		} else if (strcmp(d, "up") == 0) {
			down = false;
		} else {
			free(k);
			free(d);
			napi_throw_error(env, NULL, "Invalid key state specified.");
			return NULL;
		}
		free(d);
	}

	if (argc == 3) {
		switch (GetFlagsFromValue(env, args[2], &flags))
		{
			case -1:
				free(k);
				napi_throw_error(env, NULL, "Null pointer in key flag.");
				return NULL;
			case -2:
				free(k);
				napi_throw_error(env, NULL, "Invalid key flag specified.");
				return NULL;
		}
	}

	switch(CheckKeyCodes(k, &key))
	{
		case -1:
			free(k);
			napi_throw_error(env, NULL, "Null pointer in key code.");
			return NULL;
		case -2:
			free(k);
			napi_throw_error(env, NULL, "Invalid key code specified.");
			return NULL;
		default:
			toggleKeyCode(key, down, flags);
			microsleep(keyboardDelay);
	}

	free(k);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
}

napi_value UnicodeTap(napi_env env, napi_callback_info info)
{
	size_t argc = 1;
	napi_value args[1];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc != 1) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	int32_t value;
	napi_get_value_int32(env, args[0], &value);

	if (value != 0) {
		unicodeTap(value);

		napi_value result;
		napi_get_boolean(env, true, &result);
		return result;
	} else {
		napi_throw_error(env, NULL, "Invalid character typed.");
		return NULL;
	}
}

napi_value TypeString(napi_env env, napi_callback_info info)
{
	size_t argc = 1;
	napi_value args[1];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc != 1) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	size_t str_size;
	napi_get_value_string_utf8(env, args[0], NULL, 0, &str_size);
	char* str = (char*)malloc(str_size + 1);
	napi_get_value_string_utf8(env, args[0], str, str_size + 1, &str_size);

	typeStringDelayed(str, 0);

	free(str);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
}

napi_value TypeStringDelayed(napi_env env, napi_callback_info info)
{
	size_t argc = 2;
	napi_value args[2];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc != 2) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	size_t str_size;
	napi_get_value_string_utf8(env, args[0], NULL, 0, &str_size);
	char* str = (char*)malloc(str_size + 1);
	napi_get_value_string_utf8(env, args[0], str, str_size + 1, &str_size);

	int32_t cpm;
	napi_get_value_int32(env, args[1], &cpm);

	typeStringDelayed(str, cpm);

	free(str);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
}

napi_value SetKeyboardDelay(napi_env env, napi_callback_info info)
{
	size_t argc = 1;
	napi_value args[1];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc != 1) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	napi_get_value_int32(env, args[0], &keyboardDelay);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
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

napi_value GetPixelColor(napi_env env, napi_callback_info info) {
	size_t argc = 2;
	napi_value args[2];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc != 2) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	int32_t x, y;
	napi_get_value_int32(env, args[0], &x);
	napi_get_value_int32(env, args[1], &y);

	if (!pointVisibleOnMainDisplay(MMPointMake(x, y))) {
		napi_throw_error(env, NULL, "Requested coordinates are outside the main screen's dimensions.");
		return NULL;
	}

	MMBitmapRef bitmap = copyMMBitmapFromDisplayInRect(MMRectMake(x, y, 1, 1));
	MMRGBHex color = MMRGBHexAtPoint(bitmap, 0, 0);
	char hex[7];
	padHex(color, hex);
	destroyMMBitmap(bitmap);

	napi_value result;
	napi_create_string_utf8(env, hex, NAPI_AUTO_LENGTH, &result);
	return result;
}

napi_value GetScreenSize(napi_env env, napi_callback_info info) {
	MMSize displaySize = getMainDisplaySize();

	napi_value obj;
	napi_create_object(env, &obj);
	napi_value width, height;
	napi_create_int32(env, (int)displaySize.width, &width);
	napi_create_int32(env, (int)displaySize.height, &height);
	napi_set_named_property(env, obj, "width", width);
	napi_set_named_property(env, obj, "height", height);

	return obj;
}

napi_value GetXDisplayName(napi_env env, napi_callback_info info) {
#if defined(USE_X11)
	const char* display = getXDisplay();
	napi_value result;
	napi_create_string_utf8(env, display, NAPI_AUTO_LENGTH, &result);
	return result;
#else
	napi_throw_error(env, NULL, "getXDisplayName is only supported on Linux");
	return NULL;
#endif
}

napi_value SetXDisplayName(napi_env env, napi_callback_info info) {
#if defined(USE_X11)
	size_t argc = 1;
	napi_value args[1];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	size_t str_size;
	napi_get_value_string_utf8(env, args[0], NULL, 0, &str_size);
	char* display = (char*)malloc(str_size + 1);
	napi_get_value_string_utf8(env, args[0], display, str_size + 1, &str_size);

	setXDisplay(display);
	free(display);

	napi_value result;
	napi_get_boolean(env, true, &result);
	return result;
#else
	napi_throw_error(env, NULL, "setXDisplayName is only supported on Linux");
	return NULL;
#endif
}

napi_value CaptureScreen(napi_env env, napi_callback_info info) {
	size_t argc = 4;
	napi_value args[4];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	int32_t x, y, w, h;

	if (argc == 4) {
		napi_get_value_int32(env, args[0], &x);
		napi_get_value_int32(env, args[1], &y);
		napi_get_value_int32(env, args[2], &w);
		napi_get_value_int32(env, args[3], &h);
	} else {
		x = 0;
		y = 0;
		MMSize displaySize = getMainDisplaySize();
		w = displaySize.width;
		h = displaySize.height;
	}

	MMBitmapRef bitmap = copyMMBitmapFromDisplayInRect(MMRectMake(x, y, w, h));
	uint32_t bufferSize = bitmap->bytewidth * bitmap->height;
	napi_value buffer;
	void* data;
	napi_create_buffer_copy(env, bufferSize, bitmap->imageBuffer, &data, &buffer);

	napi_value obj;
	napi_create_object(env, &obj);
	napi_value width, height, byteWidth, bitsPerPixel, bytesPerPixel;
	napi_create_int32(env, bitmap->width, &width);
	napi_create_int32(env, bitmap->height, &height);
	napi_create_int32(env, bitmap->bytewidth, &byteWidth);
	napi_create_int32(env, bitmap->bitsPerPixel, &bitsPerPixel);
	napi_create_int32(env, bitmap->bytesPerPixel, &bytesPerPixel);
	napi_set_named_property(env, obj, "width", width);
	napi_set_named_property(env, obj, "height", height);
	napi_set_named_property(env, obj, "byteWidth", byteWidth);
	napi_set_named_property(env, obj, "bitsPerPixel", bitsPerPixel);
	napi_set_named_property(env, obj, "bytesPerPixel", bytesPerPixel);
	napi_set_named_property(env, obj, "image", buffer);

	destroyMMBitmap(bitmap);

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
class BMP
{
	public:
		uint32_t width;
		uint32_t height;
		uint32_t byteWidth;
		uint32_t bitsPerPixel;
		uint32_t bytesPerPixel;
		uint8_t *image;
};

//Convert object from Javascript to a C++ class (BMP).
BMP buildBMP(napi_env env, napi_value info)
{
	BMP img;

	napi_value width, height, byteWidth, bitsPerPixel, bytesPerPixel, image;
	napi_get_named_property(env, info, "width", &width);
	napi_get_named_property(env, info, "height", &height);
	napi_get_named_property(env, info, "byteWidth", &byteWidth);
	napi_get_named_property(env, info, "bitsPerPixel", &bitsPerPixel);
	napi_get_named_property(env, info, "bytesPerPixel", &bytesPerPixel);
	napi_get_named_property(env, info, "image", &image);

	napi_get_value_uint32(env, width, &img.width);
	napi_get_value_uint32(env, height, &img.height);
	napi_get_value_uint32(env, byteWidth, &img.byteWidth);
	napi_get_value_uint32(env, bitsPerPixel, &img.bitsPerPixel);
	napi_get_value_uint32(env, bytesPerPixel, &img.bytesPerPixel);

	void* buf;
	size_t buf_len;
	napi_get_buffer_info(env, image, &buf, &buf_len);

	// Convert the buffer to a uint8_t which createMMBitmap requires.
	img.image = (uint8_t *)malloc(img.byteWidth * img.height);
	memcpy(img.image, buf, img.byteWidth * img.height);

	return img;
}

napi_value GetColor(napi_env env, napi_callback_info info)
{
	size_t argc = 3;
	napi_value args[3];
	napi_get_cb_info(env, info, &argc, args, NULL, NULL);

	if (argc != 3) {
		napi_throw_error(env, NULL, "Invalid number of arguments.");
		return NULL;
	}

	BMP img = buildBMP(env, args[0]);

	int32_t x, y;
	napi_get_value_int32(env, args[1], &x);
	napi_get_value_int32(env, args[2], &y);

	MMBitmapRef bitmap = createMMBitmap(img.image, img.width, img.height, img.byteWidth, img.bitsPerPixel, img.bytesPerPixel);

	// Make sure the requested pixel is inside the bitmap.
	if (!MMBitmapPointInBounds(bitmap, MMPointMake(x, y))) {
		destroyMMBitmap(bitmap);
		napi_throw_error(env, NULL, "Requested coordinates are outside the bitmap's dimensions.");
		return NULL;
	}

	MMRGBHex color = MMRGBHexAtPoint(bitmap, x, y);

	char hex[7];
	padHex(color, hex);

	destroyMMBitmap(bitmap);

	napi_value result;
	napi_create_string_utf8(env, hex, NAPI_AUTO_LENGTH, &result);
	return result;
}


napi_value InitAll(napi_env env, napi_value exports) {
	napi_value fn;

	napi_create_function(env, NULL, 0, DragMouse, NULL, &fn);
	napi_set_named_property(env, exports, "dragMouse", fn);

	napi_create_function(env, NULL, 0, UpdateScreenMetrics, NULL, &fn);
	napi_set_named_property(env, exports, "updateScreenMetrics", fn);

	napi_create_function(env, NULL, 0, MoveMouse, NULL, &fn);
	napi_set_named_property(env, exports, "moveMouse", fn);

	napi_create_function(env, NULL, 0, MoveMouseSmooth, NULL, &fn);
	napi_set_named_property(env, exports, "moveMouseSmooth", fn);

	napi_create_function(env, NULL, 0, GetMousePos, NULL, &fn);
	napi_set_named_property(env, exports, "getMousePos", fn);

	napi_create_function(env, NULL, 0, MouseClick, NULL, &fn);
	napi_set_named_property(env, exports, "mouseClick", fn);

	napi_create_function(env, NULL, 0, MouseToggle, NULL, &fn);
	napi_set_named_property(env, exports, "mouseToggle", fn);

	napi_create_function(env, NULL, 0, SetMouseDelay, NULL, &fn);
	napi_set_named_property(env, exports, "setMouseDelay", fn);

	napi_create_function(env, NULL, 0, ScrollMouse, NULL, &fn);
	napi_set_named_property(env, exports, "scrollMouse", fn);

	napi_create_function(env, NULL, 0, KeyTap, NULL, &fn);
	napi_set_named_property(env, exports, "keyTap", fn);

	napi_create_function(env, NULL, 0, KeyToggle, NULL, &fn);
	napi_set_named_property(env, exports, "keyToggle", fn);

	napi_create_function(env, NULL, 0, UnicodeTap, NULL, &fn);
	napi_set_named_property(env, exports, "unicodeTap", fn);

	napi_create_function(env, NULL, 0, TypeString, NULL, &fn);
	napi_set_named_property(env, exports, "typeString", fn);

	napi_create_function(env, NULL, 0, TypeStringDelayed, NULL, &fn);
	napi_set_named_property(env, exports, "typeStringDelayed", fn);

	napi_create_function(env, NULL, 0, SetKeyboardDelay, NULL, &fn);
	napi_set_named_property(env, exports, "setKeyboardDelay", fn);

	napi_create_function(env, NULL, 0, GetPixelColor, NULL, &fn);
	napi_set_named_property(env, exports, "getPixelColor", fn);

	napi_create_function(env, NULL, 0, GetScreenSize, NULL, &fn);
	napi_set_named_property(env, exports, "getScreenSize", fn);

	napi_create_function(env, NULL, 0, GetXDisplayName, NULL, &fn);
	napi_set_named_property(env, exports, "getXDisplayName", fn);

	napi_create_function(env, NULL, 0, SetXDisplayName, NULL, &fn);
	napi_set_named_property(env, exports, "setXDisplayName", fn);

	napi_create_function(env, NULL, 0, CaptureScreen, NULL, &fn);
	napi_set_named_property(env, exports, "captureScreen", fn);

	napi_create_function(env, NULL, 0, GetColor, NULL, &fn);
	napi_set_named_property(env, exports, "getColor", fn);

	return exports;
}

NAPI_MODULE(robotjs, InitAll)
