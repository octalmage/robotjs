#include <node.h>
#include <v8.h>
#include <vector>
#include "mouse.h"
#include "deadbeef_rand.h"
#include "screen.h"
#include "screengrab.h"
#include "keypress.h"

using namespace v8;

/*
 __  __                      
|  \/  | ___  _   _ ___  ___ 
| |\/| |/ _ \| | | / __|/ _ \
| |  | | (_) | |_| \__ \  __/
|_|  |_|\___/ \__,_|___/\___|
 */

Handle<Value> moveMouse(const Arguments& args) 
{
  HandleScope scope;
  if (args.Length() < 2) 
  {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }
  size_t x = args[0]->Int32Value();
  size_t y = args[1]->Int32Value();

  MMPoint point;
  point = MMPointMake(x, y);
  moveMouse(point);
  return scope.Close(String::New("1"));
}

Handle<Value> getMousePos(const Arguments& args) 
{
  HandleScope scope;

  MMPoint pos = getMousePos();

  //Return object with .x and .y.
  Local<Object> obj = Object::New();
  obj->Set(String::NewSymbol("x"), Number::New(pos.x));
  obj->Set(String::NewSymbol("y"), Number::New(pos.y));
  return scope.Close(obj);
}

Handle<Value> mouseClick(const Arguments& args) 
{
  HandleScope scope;

  MMMouseButton button = LEFT_BUTTON;

  clickMouse(button);
  return scope.Close(String::New("1"));
}

/*
 _  __          _                         _ 
| |/ /___ _   _| |__   ___   __ _ _ __ __| |
| ' // _ \ | | | '_ \ / _ \ / _` | '__/ _` |
| . \  __/ |_| | |_) | (_) | (_| | | | (_| |
|_|\_\___|\__, |_.__/ \___/ \__,_|_|  \__,_|
          |___/           
 */

char *get(v8::Local<v8::Value> value, const char *fallback = "") 
{
    if (value->IsString()) 
    {
        v8::String::AsciiValue string(value);
        char *str = (char *) malloc(string.length() + 1);
        strcpy(str, *string);
        return str;
    }
    char *str = (char *) malloc(strlen(fallback) + 1);
    strcpy(str, fallback);
    return str;
}

Handle<Value> keyTap(const Arguments& args) 
{
  HandleScope scope;

  MMKeyFlags flags = MOD_NONE;

  char c = get(args[0])[0];
 
  if (strlen(&c)==1)
  {
    tapKey(c, flags);
  }

  return scope.Close(String::New("1"));
}

Handle<Value> typeString(const Arguments& args) 
{
  HandleScope scope;

  char *str = get(args[0]);

  typeString(str);

  return scope.Close(String::New("1"));
}

//Screen

Handle<Value> captureScreen(const Arguments& args) 
{
  HandleScope scope;

  MMRect rect;
  MMBitmapRef bitmap = NULL;
  MMSize displaySize = getMainDisplaySize();

  rect = MMRectMake(0, 0, displaySize.width, displaySize.height);

  bitmap = copyMMBitmapFromDisplayInRect(rect);

  return scope.Close(External::Wrap(bitmap));

  //return scope.Close(String::New("1"));
}

Handle<Value> getWindows(const Arguments& args) 
{
  HandleScope scope;

  CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
  CFIndex windowNum = CFArrayGetCount(windowList);

  std::vector<Object> windows;
  Local<Object> obj = Object::New();

  
  for (int i = 0; i < (int)windowNum; i++) 
  {
        CFDictionaryRef info = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);
        CFNumberRef currentPID = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowOwnerPID);
        CFNumberRef currentWindowNumber = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowNumber);
        CFStringRef currentTitle = (CFStringRef)CFDictionaryGetValue(info, kCGWindowName);
        CFIndex length = CFStringGetLength(currentTitle);
        CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
        char *buffer = (char *)malloc(maxSize);
        CFStringGetCString(currentTitle, buffer, maxSize, kCFStringEncodingUTF8);
        obj->Set(String::NewSymbol("title"), String::New(buffer));
        obj->Set(String::NewSymbol("id"), Number::New(*(int *)currentWindowNumber));
        printf(buffer);
        printf("\n");
        //windows.push_back(obj);

  }


  
  //return scope.Close(String::New("1"));

  return scope.Close(String::New("1"));
}


void init(Handle<Object> target) 
{
  target->Set(String::NewSymbol("moveMouse"),
      FunctionTemplate::New(moveMouse)->GetFunction());

  target->Set(String::NewSymbol("getMousePos"),
      FunctionTemplate::New(getMousePos)->GetFunction());

  target->Set(String::NewSymbol("mouseClick"),
      FunctionTemplate::New(mouseClick)->GetFunction());

  target->Set(String::NewSymbol("keyTap"),
      FunctionTemplate::New(keyTap)->GetFunction());

  target->Set(String::NewSymbol("typeString"),
      FunctionTemplate::New(typeString)->GetFunction());

  target->Set(String::NewSymbol("captureScreen"),
      FunctionTemplate::New(captureScreen)->GetFunction());

  target->Set(String::NewSymbol("getWindows"),
      FunctionTemplate::New(getWindows)->GetFunction());
}

NODE_MODULE(robotjs, init)
