#include "v8-internal.h"

namespace v8 {
using namespace internal;

JSUint32
Array::Length() const
{
  jsuint length;
  if (JS_GetArrayLength(cx(), *this, &length)) {
    return length;
  }

  return 0;
}

Local<Object>
Array::CloneElementAt(JSUint32 index)
{
  Local<Value> toBeCloned = Get(index);
  if (!toBeCloned->IsObject()) {
    return NULL;
  }
  return toBeCloned->ToObject()->Clone();
}

// static
Local<Array>
Array::New(int length)
{
  UNIMPLEMENTEDAPI(NULL);
}

Array::Array() :
  Object(NULL)
{
  UNIMPLEMENTEDAPI();
}

} // namespace v8
