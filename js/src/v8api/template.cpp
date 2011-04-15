#include "v8-internal.h"

namespace v8 {
using namespace internal;

Template::Template(JSClass* clasp) :
  SecretObject<Data>(JS_NewObject(cx(), clasp, NULL, NULL))
{
}

Template::Template(JSObject* obj) :
  SecretObject<Data>(obj)
{
}

void
Template::Set(Handle<String> name,
              Handle<Data> data,
              PropertyAttribute attribs)
{
  Object &obj = *reinterpret_cast<Object*>(this);
  // XXX: I feel bad about this
  Value v(data->native());
  obj.Set(name, Handle<Value>(&v), attribs);
}

void
Template::Set(const char* name,
              Handle<Data> data)
{
  Set(String::New(name), data);
}

} // namespace v8
