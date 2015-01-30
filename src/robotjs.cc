#include <node.h>
#include <nan.h>
#include <v8.h>
#include <vector>
#include "mouse.h"
#include "deadbeef_rand.h"
#include "keypress.h"

using namespace v8;

/*
 __  __                      
|  \/  | ___  _   _ ___  ___ 
| |\/| |/ _ \| | | / __|/ _ \
| |  | | (_) | |_| \__ \  __/
|_|  |_|\___/ \__,_|___/\___|

 */

NAN_METHOD(moveMouse) 
{
  NanScope();
  if (args.Length() < 2) 
  {
    return NanThrowError("Invalid number of arguments"); 
  }
  size_t x = args[0]->Int32Value();
  size_t y = args[1]->Int32Value();

  MMPoint point;
  point = MMPointMake(x, y);
  moveMouse(point);
  NanReturnValue(NanNew("1"));
}

NAN_METHOD(getMousePos) 
{
  NanScope();

  MMPoint pos = getMousePos();

  //Return object with .x and .y.
  Local<Object> obj = NanNew<Object>();
  obj->Set(NanNew<String>("x"), NanNew<Number>(pos.x));
  obj->Set(NanNew<String>("y"), NanNew<Number>(pos.y));
  NanReturnValue(obj);
}

NAN_METHOD(mouseClick) 
{
  NanScope();

  MMMouseButton button = LEFT_BUTTON;

  clickMouse(button);

  NanReturnValue(NanNew("1"));
}

/*
 _  __          _                         _ 
| |/ /___ _   _| |__   ___   __ _ _ __ __| |
| ' // _ \ | | | '_ \ / _ \ / _` | '__/ _` |
| . \  __/ |_| | |_) | (_) | (_| | | | (_| |
|_|\_\___|\__, |_.__/ \___/ \__,_|_|  \__,_|
          |___/           
 */

NAN_METHOD (keyTap) 
{
  NanScope();

  MMKeyFlags flags = MOD_NONE;
  
  const char c = (*v8::String::Utf8Value(args[0]->ToString()))[0];

  tapKey(c, flags);

  NanReturnValue(NanNew("1"));
}

NAN_METHOD (typeString) 
{
  NanScope();
  
  char *str;
  NanUtf8String string(args[0]);

  str= *string;

  typeString(str);

  NanReturnValue(NanNew("1"));
}

void init(Handle<Object> target) 
{

  target->Set(NanNew<String>("moveMouse"),
    NanNew<FunctionTemplate>(moveMouse)->GetFunction());

  target->Set(NanNew<String>("getMousePos"),
    NanNew<FunctionTemplate>(getMousePos)->GetFunction());

  target->Set(NanNew<String>("mouseClick"),
    NanNew<FunctionTemplate>(mouseClick)->GetFunction());

  target->Set(NanNew<String>("keyTap"),
    NanNew<FunctionTemplate>(keyTap)->GetFunction());

  target->Set(NanNew<String>("typeString"),
    NanNew<FunctionTemplate>(typeString)->GetFunction());

}

NODE_MODULE(robotjs, init)
