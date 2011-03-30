#include "v8-internal.h"

namespace v8 {
using namespace internal;

namespace {

JSBool
ot_GetProperty(JSContext* cx,
               JSObject* obj,
               jsid id,
               jsval* vp)
{
  // TODO We set accessors on the |Object| representation of our private data.
  //      Somehow, we need to check if that exists here, and use it if so.  For
  //      now, we just call the stub method.
  return JS_PropertyStub(cx, obj, id, vp);
}

JSBool
ot_SetProperty(JSContext* cx,
               JSObject* obj,
               jsid id,
               JSBool strict,
               jsval* vp)
{
  // TODO We set accessors on the |Object| representation of our private data.
  //      Somehow, we need to check if that exists here, and use it if so.  For
  //      now, we just call the stub method.
  return JS_StrictPropertyStub(cx, obj, id, strict, vp);
}

JSClass gNewInstanceClass = {
  NULL, // name
  JSCLASS_HAS_PRIVATE, // flags
  JS_PropertyStub, // addProperty
  JS_PropertyStub, // delProperty
  ot_GetProperty, // getProperty
  ot_SetProperty, // setProperty
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

struct PrivateData
{
  static PrivateData* Get(JSContext* cx,
                          JSObject* obj)
  {
    return static_cast<PrivateData*>(JS_GetPrivate(cx, obj));
  }
  static PrivateData* Get(jsval val)
  {
    JSObject* obj = JSVAL_TO_OBJECT(val);
    return static_cast<PrivateData*>(JS_GetPrivate(cx(), obj));
  }
};

void
finalize(JSContext* cx,
         JSObject* obj)
{
  PrivateData* data = PrivateData::Get(cx, obj);
  delete data;
}

JSClass gObjectTemplateClass = {
  "ObjectTemplate", // name
  JSCLASS_HAS_PRIVATE, // flags
  JS_PropertyStub, // addProperty
  JS_PropertyStub, // delProperty
  JS_PropertyStub, // getProperty
  JS_StrictPropertyStub, // setProperty
  JS_EnumerateStub, // enumerate
  JS_ResolveStub, // resolve
  JS_ConvertStub, // convert
  finalize, // finalize
  NULL, // getObjectOps
  NULL, // checkAccess
  NULL, // call
  NULL, // construct
  NULL, // xdrObject
  NULL, // hasInstance
  NULL, // mark
  NULL, // reservedSlots
};

} // anonymous namespace

ObjectTemplate::ObjectTemplate() :
  Template(&gObjectTemplateClass)
{
  PrivateData* data = new PrivateData();
  (void)JS_SetPrivate(cx(), JSVAL_TO_OBJECT(mVal), data);
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
  // We've set everything we care about on our SecretObject, so we can assign
  // that to the prototype of our new object.
  JSObject* obj = JS_NewObject(cx(), &gNewInstanceClass, InternalObject(), NULL);
  if (!obj) {
    return NULL;
  }

  if (!JS_SetPrivate(cx(), obj, this)) {
    // TODO handle error better
    return NULL;
  }

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
  (void)InternalObject().SetAccessor(name, getter, setter, data, settings,
                                     attribute);
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
