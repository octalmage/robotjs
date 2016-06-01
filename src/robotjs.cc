#include <node.h>
#include <nan.h>
#include <v8.h>
#include <vector>
#include "mouse.h"
#include "deadbeef_rand.h"
#include "keypress.h"
#include "screen.h"
#include "screengrab.h"
#include "MMBitmap.h"
#include "snprintf.h"
#include "microsleep.h"

using namespace v8;

//Global delays.
int mouseDelay = 10;
int keyboardDelay = 10;

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

NAN_METHOD(dragMouse)
{
	if (info.Length() < 2 || info.Length() > 3)
	{
		return Nan::ThrowError("Invalid number of arguments.");
	}

	const size_t x = info[0]->Int32Value();
	const size_t y = info[1]->Int32Value();
	MMMouseButton button = LEFT_BUTTON;

	if (info.Length() == 3)
	{
		Nan::Utf8String bstr(info[2]);
		const char * const b = *bstr;

		switch (CheckMouseButton(b, &button))
		{
			case -1:
				return Nan::ThrowError("Null pointer in mouse button code.");
				break;
			case -2:
				return Nan::ThrowError("Invalid mouse button specified.");
				break;
		}
	}

	MMPoint point;
	point = MMPointMake(x, y);
	dragMouse(point, button);
	microsleep(mouseDelay);

	info.GetReturnValue().Set(Nan::New(1));
}

NAN_METHOD(moveMouse)
{
	if (info.Length() != 2)
	{
		return Nan::ThrowError("Invalid number of arguments.");
	}
	size_t x = info[0]->Int32Value();
	size_t y = info[1]->Int32Value();

	MMPoint point;
	point = MMPointMake(x, y);
	moveMouse(point);
	microsleep(mouseDelay);

	info.GetReturnValue().Set(Nan::New(1));
}

NAN_METHOD(moveMouseSmooth)
{
	if (info.Length() != 2)
	{
		return Nan::ThrowError("Invalid number of arguments.");
	}
	size_t x = info[0]->Int32Value();
	size_t y = info[1]->Int32Value();

	MMPoint point;
	point = MMPointMake(x, y);
	smoothlyMoveMouse(point);
	microsleep(mouseDelay);

	info.GetReturnValue().Set(Nan::New(1));
}

NAN_METHOD(getMousePos)
{
	MMPoint pos = getMousePos();

	//Return object with .x and .y.
	Local<Object> obj = Nan::New<Object>();
	Nan::Set(obj, Nan::New("x").ToLocalChecked(), Nan::New((int)pos.x));
	Nan::Set(obj, Nan::New("y").ToLocalChecked(), Nan::New((int)pos.y));
	info.GetReturnValue().Set(obj);
}

NAN_METHOD(mouseClick)
{
	MMMouseButton button = LEFT_BUTTON;
	bool doubleC = false;

	if (info.Length() > 0)
	{
		v8::String::Utf8Value bstr(info[0]->ToString());
		const char * const b = *bstr;

		switch (CheckMouseButton(b, &button))
		{
			case -1:
				return Nan::ThrowError("Null pointer in mouse button code.");
				break;
			case -2:
				return Nan::ThrowError("Invalid mouse button specified.");
				break;
		}
	}

	if (info.Length() == 2)
	{
		doubleC = info[1]->BooleanValue();
	}
	else if (info.Length() > 2)
	{
		return Nan::ThrowError("Invalid number of arguments.");
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

	info.GetReturnValue().Set(Nan::New(1));
}

NAN_METHOD(mouseToggle)
{
	MMMouseButton button = LEFT_BUTTON;
	bool down = false;

	if (info.Length() > 0)
	{
		char *d;

		Nan::Utf8String dstr(info[0]);
		d = *dstr;

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
			return Nan::ThrowError("Invalid mouse button state specified.");
		}
	}

	if (info.Length() == 2)
	{
		Nan::Utf8String bstr(info[1]);
		const char * const b = *bstr;

		switch (CheckMouseButton(b, &button))
		{
			case -1:
				return Nan::ThrowError("Null pointer in mouse button code.");
				break;
			case -2:
				return Nan::ThrowError("Invalid mouse button specified.");
				break;
		}
	}
	else if (info.Length() > 2)
	{
		return Nan::ThrowError("Invalid number of arguments.");
	}

	toggleMouse(down, button);
	microsleep(mouseDelay);

	info.GetReturnValue().Set(Nan::New(1));
}

NAN_METHOD(setMouseDelay)
{
	if (info.Length() != 1)
	{
		return Nan::ThrowError("Invalid number of arguments.");
	}

	mouseDelay = info[0]->Int32Value();

	info.GetReturnValue().Set(Nan::New(1));
}

NAN_METHOD(scrollMouse) 
{
	Nan::HandleScope scope;

	//Get the values of magnitude and direction from the arguments list.
	if(info.Length() == 2)	
	{
		int scrollMagnitude = info[0]->Int32Value();
		char *s;

		Nan::Utf8String sstr(info[1]);
		s = *sstr;
		
		MMMouseWheelDirection scrollDirection;
		
		if (strcmp(s, "up") == 0)
		{
			scrollDirection = DIRECTION_UP;
		}
		else if (strcmp(s, "down") == 0)
		{
			scrollDirection = DIRECTION_DOWN;
		}
		else
		{
			return Nan::ThrowError("Invalid scroll direction specified.");
		}
		
		scrollMouse(scrollMagnitude, scrollDirection);
		microsleep(mouseDelay);
		
		info.GetReturnValue().Set(Nan::New(1));
	} 
	else 
	{
		return Nan::ThrowError("Invalid number of arguments.");
	}
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
	{ "command",        K_META },
	{ "alt",            K_ALT },
	{ "control",        K_CONTROL },
	{ "shift",          K_SHIFT },
	{ "right_shift",    K_RIGHTSHIFT },
	{ "space",          K_SPACE },
	{ "printscreen",    K_PRINTSCREEN },
	{ "insert",         K_INSERT },
				  
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
				   
	{ "lights_mon_up",    K_LIGHTS_MON_UP },
	{ "lights_mon_down",  K_LIGHTS_MON_DOWN },
	{ "lights_kbd_toggle",K_LIGHTS_KBD_TOGGLE },
	{ "lights_kbd_up",    K_LIGHTS_KBD_UP },
	{ "lights_kbd_down",  K_LIGHTS_KBD_DOWN },

	{ NULL,               K_NOT_A_KEY } /* end marker */
};

int CheckKeyCodes(char* k, MMKeyCode *key)
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

int CheckKeyFlags(char* f, MMKeyFlags* flags)
{
	if (!flags) return -1;

	if (strcmp(f, "alt") == 0)
	{
		*flags = MOD_ALT;
	}
	else if(strcmp(f, "command") == 0)
	{
		*flags = MOD_META;
	}
	else if(strcmp(f, "control") == 0)
	{
		*flags = MOD_CONTROL;
	}
	else if(strcmp(f, "shift") == 0)
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

int GetFlagsFromString(v8::Handle<v8::Value> value, MMKeyFlags* flags)
{
	v8::String::Utf8Value fstr(value->ToString());
	return CheckKeyFlags(*fstr, flags);
}

int GetFlagsFromValue(v8::Handle<v8::Value> value, MMKeyFlags* flags)
{
	if (!flags) return -1;

	//Optionally allow an array of flag strings to be passed.
	if (value->IsArray())
	{
		v8::Handle<v8::Array> a = v8::Handle<v8::Array>::Cast(value);
		for (uint32_t i = 0; i < a->Length(); i++)
		{
			v8::Handle<v8::Value> v(a->Get(i));
			if (!v->IsString()) return -2;

			MMKeyFlags f = MOD_NONE;
			const int rv = GetFlagsFromString(v, &f);
			if (rv) return rv;

			*flags = (MMKeyFlags)(*flags | f);
		}
		return 0;
	}

	//If it's not an array, it should be a single string value.
	return GetFlagsFromString(value, flags);
}

NAN_METHOD(keyTap)
{
	MMKeyFlags flags = MOD_NONE;
	MMKeyCode key;

	char *k;

	v8::String::Utf8Value kstr(info[0]->ToString());
	k = *kstr;

	switch (info.Length())
	{
		case 2:
			switch (GetFlagsFromValue(info[1], &flags))
			{
				case -1:
					return Nan::ThrowError("Null pointer in key flag.");
					break;
				case -2:
					return Nan::ThrowError("Invalid key flag specified.");
					break;
			}
			break;
		case 1:
			break;
		default:
			return Nan::ThrowError("Invalid number of arguments.");
	}

	switch(CheckKeyCodes(k, &key))
	{
		case -1:
			return Nan::ThrowError("Null pointer in key code.");
			break;
		case -2:
			return Nan::ThrowError("Invalid key code specified.");
			break;
		default:
			tapKeyCode(key, flags);
			microsleep(keyboardDelay);
	}

	info.GetReturnValue().Set(Nan::New(1));
}


NAN_METHOD(keyToggle)
{
	MMKeyFlags flags = MOD_NONE;
	MMKeyCode key;

	bool down;
	char *k;

	//Get arguments from JavaScript.
	Nan::Utf8String kstr(info[0]);

	//Convert arguments to chars.
	k = *kstr;

	//Check and confirm number of arguments.
	switch (info.Length())
	{
		case 3:
			//Get key modifier.
			switch (GetFlagsFromValue(info[2], &flags))
			{
				case -1:
					return Nan::ThrowError("Null pointer in key flag.");
					break;
				case -2:
					return Nan::ThrowError("Invalid key flag specified.");
					break;
			}
			break;
		case 2:
			break;
		default:
			return Nan::ThrowError("Invalid number of arguments.");
	}

	//Get down value if provided.
	if (info.Length() > 1)
	{
		char *d;

		Nan::Utf8String dstr(info[1]);
		d = *dstr;

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
			return Nan::ThrowError("Invalid key state specified.");
		}
	}

	//Get the acutal key.
	switch(CheckKeyCodes(k, &key))
	{
		case -1:
			return Nan::ThrowError("Null pointer in key code.");
			break;
		case -2:
			return Nan::ThrowError("Invalid key code specified.");
			break;
		default:
			toggleKeyCode(key, down, flags);
			  microsleep(keyboardDelay);
	}

	info.GetReturnValue().Set(Nan::New(1));
}

NAN_METHOD(typeString)
{
	char *str;
	Nan::Utf8String string(info[0]);

	str = *string;

	typeString(str);

	info.GetReturnValue().Set(Nan::New(1));
}

NAN_METHOD(typeStringDelayed)
{
	char *str;
	Nan::Utf8String string(info[0]);
	
	str = *string;
	
	size_t cpm = info[1]->Int32Value();

	typeStringDelayed(str, cpm);

	info.GetReturnValue().Set(Nan::New(1));
}

NAN_METHOD(setKeyboardDelay)
{
	if (info.Length() != 1)
	{
		return Nan::ThrowError("Invalid number of arguments.");
	}

	keyboardDelay = info[0]->Int32Value();

	info.GetReturnValue().Set(Nan::New(1));
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

NAN_METHOD(getPixelColor)
{
	if (info.Length() != 2)
	{
		return Nan::ThrowError("Invalid number of arguments.");
	}
	
	MMBitmapRef bitmap;
	MMRGBHex color;

	size_t x = info[0]->Int32Value();
	size_t y = info[1]->Int32Value();

	if (!pointVisibleOnMainDisplay(MMPointMake(x, y)))
	{
		return Nan::ThrowError("Requested coordinates are outside the main screen's dimensions.");
	}

	bitmap = copyMMBitmapFromDisplayInRect(MMRectMake(x, y, 1, 1));

	color = MMRGBHexAtPoint(bitmap, 0, 0);
	
	char hex[7];
	
	padHex(color, hex);

	destroyMMBitmap(bitmap);

	info.GetReturnValue().Set(Nan::New(hex).ToLocalChecked());
}

NAN_METHOD(getScreenSize)
{
	//Get display size.
	MMSize displaySize = getMainDisplaySize();

	//Create our return object.
	Local<Object> obj = Nan::New<Object>();
	Nan::Set(obj, Nan::New("width").ToLocalChecked(), Nan::New<Number>(displaySize.width));
	Nan::Set(obj, Nan::New("height").ToLocalChecked(), Nan::New<Number>(displaySize.height));

	//Return our object with .width and .height.
	info.GetReturnValue().Set(obj);
}

NAN_METHOD(captureScreen) 
{
	size_t x;
	size_t y;
	size_t w;
	size_t h;
	
	//If user has provided screen coords, use them! 
	if (info.Length() == 4)
	{
		//TODO: Make sure requested coords are within the screen bounds, or we get a seg fault.
		// 		An error message is much nicer! 

		x = info[0]->Int32Value();
		y = info[1]->Int32Value();
		w = info[2]->Int32Value();
		h = info[3]->Int32Value();
	}
	else
	{
		//We're getting the full screen.
		x = 0;
		y = 0;
		
		//Get screen size.
		MMSize displaySize = getMainDisplaySize();
		w = displaySize.width;
		h = displaySize.height;
	}
	
	MMBitmapRef bitmap = copyMMBitmapFromDisplayInRect(MMRectMake(x, y, w, h));
	
	uint32_t bufferSize = bitmap->bytewidth * bitmap->height;
	Local<Object> buffer = Nan::NewBuffer((char*)bitmap->imageBuffer, bufferSize, destroyMMBitmapBuffer, NULL).ToLocalChecked();

	Local<Object> obj = Nan::New<Object>();
	Nan::Set(obj, Nan::New("width").ToLocalChecked(), Nan::New<Number>(bitmap->width));
	Nan::Set(obj, Nan::New("height").ToLocalChecked(), Nan::New<Number>(bitmap->height));
	Nan::Set(obj, Nan::New("byteWidth").ToLocalChecked(), Nan::New<Number>(bitmap->bytewidth));
	Nan::Set(obj, Nan::New("bitsPerPixel").ToLocalChecked(), Nan::New<Number>(bitmap->bitsPerPixel));
	Nan::Set(obj, Nan::New("bytesPerPixel").ToLocalChecked(), Nan::New<Number>(bitmap->bytesPerPixel));
	Nan::Set(obj, Nan::New("image").ToLocalChecked(), buffer);
	
	info.GetReturnValue().Set(obj);
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
		size_t width;
		size_t height;
		size_t byteWidth;
		uint8_t bitsPerPixel;
		uint8_t bytesPerPixel;
		uint8_t *image;
};

//Convert object from Javascript to a C++ class (BMP).
BMP buildBMP(Local<Object> info) 
{
	Local<Object> obj = Nan::To<v8::Object>(info).ToLocalChecked();

	BMP img;

	img.width = obj->Get(Nan::New("width").ToLocalChecked())->Uint32Value();
	img.height = obj->Get(Nan::New("height").ToLocalChecked())->Uint32Value();
	img.byteWidth = obj->Get(Nan::New("byteWidth").ToLocalChecked())->Uint32Value();
	img.bitsPerPixel = obj->Get(Nan::New("bitsPerPixel").ToLocalChecked())->Uint32Value();
	img.bytesPerPixel = obj->Get(Nan::New("bytesPerPixel").ToLocalChecked())->Uint32Value();

	char* buf = node::Buffer::Data(obj->Get(Nan::New("image").ToLocalChecked()));

	//Convert the buffer to a uint8_t which createMMBitmap requires.
	img.image = (uint8_t *)malloc(img.byteWidth * img.height);
	memcpy(img.image, buf, img.byteWidth * img.height);

	return img;
 }

NAN_METHOD(getColor)
{
	MMBitmapRef bitmap;
	MMRGBHex color;

	size_t x = info[1]->Int32Value();
	size_t y = info[2]->Int32Value();

	//Get our image object from JavaScript.
	BMP img = buildBMP(Nan::To<v8::Object>(info[0]).ToLocalChecked());

	//Create the bitmap.
	bitmap = createMMBitmap(img.image, img.width, img.height, img.byteWidth, img.bitsPerPixel, img.bytesPerPixel);

	// Make sure the requested pixel is inside the bitmap.
	if (!MMBitmapPointInBounds(bitmap, MMPointMake(x, y)))
	{
		return Nan::ThrowError("Requested coordinates are outside the bitmap's dimensions.");
	}

	color = MMRGBHexAtPoint(bitmap, x, y);

	char hex[7];

	padHex(color, hex);

	destroyMMBitmap(bitmap);

	info.GetReturnValue().Set(Nan::New(hex).ToLocalChecked());

}

NAN_MODULE_INIT(InitAll)
{
	Nan::Set(target, Nan::New("dragMouse").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(dragMouse)).ToLocalChecked());

	Nan::Set(target, Nan::New("moveMouse").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(moveMouse)).ToLocalChecked());

	Nan::Set(target, Nan::New("moveMouseSmooth").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(moveMouseSmooth)).ToLocalChecked());

	Nan::Set(target, Nan::New("getMousePos").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(getMousePos)).ToLocalChecked());

	Nan::Set(target, Nan::New("mouseClick").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(mouseClick)).ToLocalChecked());

	Nan::Set(target, Nan::New("mouseToggle").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(mouseToggle)).ToLocalChecked());
		
	Nan::Set(target, Nan::New("scrollMouse").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(scrollMouse)).ToLocalChecked());

	Nan::Set(target, Nan::New("setMouseDelay").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(setMouseDelay)).ToLocalChecked());

	Nan::Set(target, Nan::New("keyTap").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(keyTap)).ToLocalChecked());

	Nan::Set(target, Nan::New("keyToggle").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(keyToggle)).ToLocalChecked());

	Nan::Set(target, Nan::New("typeString").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(typeString)).ToLocalChecked());

	Nan::Set(target, Nan::New("typeStringDelayed").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(typeStringDelayed)).ToLocalChecked());

	Nan::Set(target, Nan::New("setKeyboardDelay").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(setKeyboardDelay)).ToLocalChecked());

	Nan::Set(target, Nan::New("getPixelColor").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(getPixelColor)).ToLocalChecked());

	Nan::Set(target, Nan::New("getScreenSize").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(getScreenSize)).ToLocalChecked());
		
	Nan::Set(target, Nan::New("captureScreen").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(captureScreen)).ToLocalChecked());
		
	Nan::Set(target, Nan::New("getColor").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(getColor)).ToLocalChecked());
}

NODE_MODULE(robotjs, InitAll)
