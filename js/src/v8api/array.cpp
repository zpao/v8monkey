#include "v8-internal.h"

namespace v8 {

JSUint32
Array::Length() const
{
  UNIMPLEMENTEDAPI(0);
}

Local<Object>
Array::CloneElementAt(JSUint32 index)
{
  UNIMPLEMENTEDAPI(NULL);
}

// static
Local<Array>
Array::New(int length)
{
  UNIMPLEMENTEDAPI(NULL);
}

// static
Array*
Array::Cast(Value* obj)
{
  UNIMPLEMENTEDAPI(NULL);
}

Array::Array() :
  Object(NULL)
{
  UNIMPLEMENTEDAPI();
}

} // namespace v8
