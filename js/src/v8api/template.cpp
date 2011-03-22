#include "v8-internal.h"

namespace v8 {

void
Template::Set(Handle<String> name,
              Handle<Data> value,
              PropertyAttribute attribs)
{
  UNIMPLEMENTEDAPI();
}

void
Template::Set(const char* name,
              Handle<Data> value)
{
  Set(String::New(name), value);
}

} // namespace v8
