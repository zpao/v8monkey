#include "v8-internal.h"

namespace v8 {
namespace internal {

#ifdef DEBUG
void
Dump(Handle<Object> obj)
{
  js_DumpObject(**obj);
}

void
Dump(Object& obj)
{
  Local<Object> o = Local<Object>::New(&obj);
  Dump(o);
}

void
Dump(Handle<String> str)
{
  jsval v = str->native();
  js_DumpString(JSVAL_TO_STRING(v));
}

void
Dump(String& str)
{
  Local<String> s = Local<String>::New(&str);
  Dump(s);
}
#endif // DEBUG

} // namespace internal
} // namespace v8
