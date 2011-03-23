#include "v8-internal.h"

namespace v8 {
using namespace internal;

namespace {

JSBool
v8api_GetProperty(JSContext* cx,
                  JSObject* obj,
                  jsid id,
                  jsval* vp)
{
  // TODO this is just a stub
  return JS_PropertyStub(cx, obj, id, vp);
}

JSBool
v8api_SetProperty(JSContext* cx,
                  JSObject* obj,
                  jsid id,
                  JSBool strict,
                  jsval* vp)
{
  // TODO this is just a stub
  return JS_StrictPropertyStub(cx, obj, id, strict, vp);
}

} // anonymous namespace

ObjectTemplate::ObjectTemplate() :
  Template()
{
  // TODO we don't really want to use all these stubs...
  JSClass member = {
    NULL, // name
    0, // flags
    JS_PropertyStub, // addProperty
    JS_PropertyStub, // delProperty
    v8api_GetProperty, // getProperty
    v8api_SetProperty, // setProperty
    JS_EnumerateStub, // enumerate
    JS_ResolveStub, // resolve
    JS_ConvertStub, // convert
    JS_FinalizeStub, // finalize
    NULL, // getObjectOps
    NULL, // checkAccess
    NULL, // call
    NULL, // construct
    NULL, // xdrObject
    NULL, // hasInstance
    NULL, // mark
    NULL, // reservedSlots
  };
  mClass = member;
}

// static
Local<ObjectTemplate>
ObjectTemplate::New()
{
  UNIMPLEMENTEDAPI(NULL);
}

Local<Object>
ObjectTemplate::NewInstance()
{
  JSObject* obj = JS_NewObject(cx(), &mClass, NULL, NULL);
  if (!obj) {
    return NULL;
  }

  // TODO iterate properties added with Template::Set

  Object o(obj);
  return Local<Object>::New(&o);
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
