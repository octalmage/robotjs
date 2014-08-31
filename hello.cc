#include <node.h>
#include <v8.h>
#include "mouse.h"
#include "deadbeef_rand.h"
#include "screen.h"

using namespace v8;

Handle<Value> Method(const Arguments& args) {
  HandleScope scope;
  MMPoint point;

  point = MMPointMake(5, 5);

  moveMouse(point);
  return scope.Close(String::New("world"));
}

void init(Handle<Object> target) {
  target->Set(String::NewSymbol("hello"),
      FunctionTemplate::New(Method)->GetFunction());
}
NODE_MODULE(autonode, init)
