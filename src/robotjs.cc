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
	if (info.Length() < 2)
	{
		return Nan::ThrowError("Invalid number of arguments.");
	}

	const size_t x = info[0]->Int32Value();
	const size_t y = info[1]->Int32Value();
	MMMouseButton button = LEFT_BUTTON;

	if (info.Length() >= 3)
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
	if (info.Length() < 2)
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
	if (info.Length() < 2)
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

int CheckKeyCodes(char* k, MMKeyCode *key)
{
	if (!key) return -1;

	if (strcmp(k, "alt") == 0)
	{
		*key = K_ALT;
	}
  	else if (strcmp(k, "command") == 0)
	{
		*key = K_META;
	}
  	else if (strcmp(k, "control") == 0)
	{
		*key = K_CONTROL;
	}
  	else if (strcmp(k, "shift") == 0)
	{
		*key = K_SHIFT;
	}
	else if (strcmp(k, "backspace") == 0)
	{
		*key = K_BACKSPACE;
	}
	else if (strcmp(k, "enter") == 0)
	{
		*key = K_RETURN;
	}
	else if (strcmp(k, "tab") == 0)
	{
		*key = K_TAB;
	}
	else if (strcmp(k, "up") == 0)
	{
		*key = K_UP;
	}
	else if (strcmp(k, "down") == 0)
	{
		*key = K_DOWN;
	}
	else if (strcmp(k, "left") == 0)
	{
		*key = K_LEFT;
	}
	else if (strcmp(k, "right") == 0)
	{
		*key = K_RIGHT;
	}
	else if (strcmp(k, "escape") == 0)
	{
		*key = K_ESCAPE;
	}
	else if (strcmp(k, "delete") == 0)
	{
		*key = K_DELETE;
	}
	else if (strcmp(k, "home") == 0)
	{
		*key = K_HOME;
	}
	else if (strcmp(k, "end") == 0)
	{
		*key = K_END;
	}
	else if (strcmp(k, "pageup") == 0)
	{
		*key = K_PAGEUP;
	}
	else if (strcmp(k, "pagedown") == 0)
	{
		*key = K_PAGEDOWN;
	}
	else if (strcmp(k, "space") == 0)
	{
		*key = K_SPACE;
	}
	else if (strcmp(k, "f1") == 0)
	{
		*key = K_F1;
	}
	else if (strcmp(k, "f2") == 0)
	{
		*key = K_F2;
	}
	else if (strcmp(k, "f3") == 0)
	{
		*key = K_F3;
	}
	else if (strcmp(k, "f4") == 0)
	{
		*key = K_F4;
	}
	else if (strcmp(k, "f5") == 0)
	{
		*key = K_F5;
	}
	else if (strcmp(k, "f6") == 0)
	{
		*key = K_F6;
	}
	else if (strcmp(k, "f7") == 0)
	{
		*key = K_F7;
	}
	else if (strcmp(k, "f8") == 0)
	{
		*key = K_F8;
	}
	else if (strcmp(k, "f9") == 0)
	{
		*key = K_F9;
	}
	else if (strcmp(k, "f10") == 0)
	{
		*key = K_F10;
	}
	else if (strcmp(k, "f11") == 0)
	{
		*key = K_F11;
	}
	else if (strcmp(k, "f12") == 0)
	{
		*key = K_F12;
	}
	else if (strcmp(k, "printscreen") == 0)
	{
		#if defined(IS_WINDOWS)
			*key = K_PRINTSCREEN;
		#else
	 		Nan::ThrowError("printscreen is only supported on Windows.");
	 	#endif
	}
	else if (strlen(k) == 1)
	{
		*key = keyCodeForChar(*k);
	}
	else
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

NAN_METHOD(keyTap)
{
	MMKeyFlags flags = MOD_NONE;
	MMKeyCode key;

  	char *k;
  	char *f;

  	v8::String::Utf8Value fstr(info[1]->ToString());
  	v8::String::Utf8Value kstr(info[0]->ToString());
  	k = *kstr;
  	f = *fstr;

	switch (info.Length())
	{
		case 2:
			break;
		case 1:
			f = NULL;
			break;
		default:
			return Nan::ThrowError("Invalid number of arguments.");
	}

  	if (f)
	{
		switch(CheckKeyFlags(f, &flags))
    	{
			case -1:
				return Nan::ThrowError("Null pointer in key flag.");
        		break;
			case -2:
				return Nan::ThrowError("Invalid key flag specified.");
        		break;
    	}
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
	char *f;
	
	//Get arguments from JavaScript.
	Nan::Utf8String kstr(info[0]);
	Nan::Utf8String fstr(info[2]);

	//Convert arguments to chars.
	k = *kstr;
	f = *fstr;
	
	//Check and confirm number of arguments.
	switch (info.Length())
	{
    	case 3:
      		break;
    	case 2:
      		f = NULL;
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

	//Get key modifier.
	if (f)
	{
		switch(CheckKeyFlags(f, &flags))
		{
			case -1:
        		return Nan::ThrowError("Null pointer in key flag.");
        		break;
			case -2:
        		return Nan::ThrowError("Invalid key flag specified.");
        		break;
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

NAN_METHOD(getPixelColor)
{
	MMBitmapRef bitmap;
	MMRGBHex color;

	size_t x = info[0]->Int32Value();
	size_t y = info[1]->Int32Value();

	bitmap = copyMMBitmapFromDisplayInRect(MMRectMake(x, y, 1, 1));

	color = MMRGBHexAtPoint(bitmap, 0, 0);

	char hex [7];

	//Length needs to be 7 because snprintf includes a terminating null.
	//Use %06x to pad hex value with leading 0s.
	snprintf(hex, 7, "%06x", color);

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

    Nan::Set(target, Nan::New("setKeyboardDelay").ToLocalChecked(),
        Nan::GetFunction(Nan::New<FunctionTemplate>(setKeyboardDelay)).ToLocalChecked());

    Nan::Set(target, Nan::New("getPixelColor").ToLocalChecked(),
        Nan::GetFunction(Nan::New<FunctionTemplate>(getPixelColor)).ToLocalChecked());

    Nan::Set(target, Nan::New("getScreenSize").ToLocalChecked(),
        Nan::GetFunction(Nan::New<FunctionTemplate>(getScreenSize)).ToLocalChecked());
}

NODE_MODULE(robotjs, InitAll)
