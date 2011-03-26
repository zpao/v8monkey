#include "v8-internal.h"

namespace v8 {
using namespace internal;

Template::Template() :
  Data(OBJECT_TO_JSVAL(JS_NewObject(cx(), NULL, NULL, NULL)))
{
}

void
Template::Set(Handle<String> name,
              Handle<Value> value,
              PropertyAttribute attribs)
{
  Object &obj = *reinterpret_cast<Object*>(this);
  obj.Set(name, value, attribs);
}

void
Template::Set(const char* name,
              Handle<Value> value)
{
  Set(String::New(name), value);
}

} // namespace v8
