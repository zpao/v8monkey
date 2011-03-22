#include "v8-internal.h"

namespace v8 {
using namespace internal;

// static
Local<ObjectTemplate>
ObjectTemplate::New()
{
  UNIMPLEMENTEDAPI(NULL);
}

Local<Object>
ObjectTemplate::NewInstance()
{
  UNIMPLEMENTEDAPI(NULL);
}

void
ObjectTemplate::SetAccessor(Handle<String> name,
                            AccessorGetter getter,
                            AccessorSetter setter,
                            Handle<Value> data,
                            AccessControl settings,
                            PropertyAttribute attribute)
{
  UNIMPLEMENTEDAPI();
}

void
ObjectTemplate::SetNamedPropertyHandler(NamedPropertyGetter getter,
                                        NamedPropertySetter setter,
                                        NamedPropertyQuery query,
                                        NamedPropertyDeleter deleter,
                                        NamedPropertyEnumerator enumerator,
                                        Handle<Value> data)
{
  UNIMPLEMENTEDAPI();
}

void
ObjectTemplate::SetIndexedPropertyHandler(IndexedPropertyGetter getter,
                                          IndexedPropertySetter setter,
                                          IndexedPropertyQuery query,
                                          IndexedPropertyDeleter deleter,
                                          IndexedPropertyEnumerator enumerator,
                                          Handle<Value> data)
{
  UNIMPLEMENTEDAPI();
}

void
ObjectTemplate::SetCallAsFunctionHandler(InvocationCallback callback,
                                         Handle<Value> data)
{
  UNIMPLEMENTEDAPI();
}

void
ObjectTemplate::MarkAsUndetectable()
{
  UNIMPLEMENTEDAPI();
}

void
ObjectTemplate::SetAccessCheckCallbacks(NamedSecurityCallback named_handler,
                                        IndexedSecurityCallback indexed_handler,
                                        Handle<Value> data,
                                        bool turned_on_by_default)
{
  UNIMPLEMENTEDAPI();
}

int
ObjectTemplate::InternalFieldCount()
{
  UNIMPLEMENTEDAPI(0);
}

void
ObjectTemplate::SetInternalFieldCount(int value)
{
  UNIMPLEMENTEDAPI();
}

} // namespace v8
