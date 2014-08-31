#include <node.h>
#include <v8.h>
#include "mouse.h"
#include "deadbeef_rand.h"
#include "screen.h"

using namespace v8;

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
  Local<Object> obj = Object::New();
  obj->Set(String::NewSymbol("x"), Number::New(pos.x));
  obj->Set(String::NewSymbol("y"), Number::New(pos.y));
  return scope.Close(obj);
}

void init(Handle<Object> target) 
{
  target->Set(String::NewSymbol("moveMouse"),
      FunctionTemplate::New(moveMouse)->GetFunction());

  target->Set(String::NewSymbol("getMousePos"),
      FunctionTemplate::New(getMousePos)->GetFunction());
}
NODE_MODULE(autonode, init)
