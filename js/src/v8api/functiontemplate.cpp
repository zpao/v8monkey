#include "v8-internal.h"

namespace v8 {
using namespace internal;

JS_STATIC_ASSERT(sizeof(FunctionTemplate) == sizeof(GCReference));

namespace {
const int kInstanceSlot = 0;
const int kDataSlot = 1;
const int kCallbackSlot = 2;
const int kCallbackParitySlot = 3;
const int kFunctionTemplateSlots = 4;
} // anonymous namespace

JSClass FunctionTemplate::sFunctionTemplateClass = {
  "FunctionTemplate", // name
  JSCLASS_HAS_RESERVED_SLOTS(kFunctionTemplateSlots), // flags
  JS_PropertyStub, // addProperty
  JS_PropertyStub, // delProperty
  JS_PropertyStub, // getProperty
  JS_StrictPropertyStub, // setProperty
  JS_EnumerateStub, // enumerate
  JS_ResolveStub, // resolve
  JS_ConvertStub, // convert
  JS_FinalizeStub, // finalize
  NULL, // unused
  NULL, // checkAccess
  NULL, // call
  NULL, // construct
  NULL, // xdrObject
  NULL, // hasInstance
  NULL, // trace
};

FunctionTemplate::FunctionTemplate() :
  Template(&sFunctionTemplateClass)
{
  Handle<ObjectTemplate> protoTemplate = ObjectTemplate::New();
  Set("prototype", protoTemplate);
  // Instance template
  Local<ObjectTemplate> instanceTemplate = ObjectTemplate::New();
  instanceTemplate->SetPrototype(protoTemplate);

  InternalObject().SetInternalField(kInstanceSlot, Handle<Object>(&instanceTemplate->InternalObject()));
  InternalObject().SetInternalField(kDataSlot, v8::Undefined());
  InternalObject().SetPointerInInternalField(kCallbackSlot, NULL);
  InternalObject().SetPointerInInternalField(kCallbackParitySlot, NULL);
}

JSBool
FunctionTemplate::CallCallback(JSContext* cx,
                               uintN argc,
                               jsval* vp)
{
  Object fnTemplateObj(JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)));
  Handle<Object> ftData = fnTemplateObj.GetInternalField(0).As<Object>();
  Handle<ObjectTemplate> instanceTemplate =
    reinterpret_cast<ObjectTemplate*>(*ftData->GetInternalField(kInstanceSlot));
  JSIntPtr maskedCallback = reinterpret_cast<JSIntPtr>(
    ftData->GetPointerFromInternalField(kCallbackSlot)
  );
  JSIntPtr residual = reinterpret_cast<JSIntPtr>(
    ftData->GetPointerFromInternalField(kCallbackParitySlot)
  );
  InvocationCallback callback = reinterpret_cast<InvocationCallback>(
    maskedCallback | (residual >> 1)
  );
  Local<Value> data = ftData->GetInternalField(kDataSlot);

  JSObject* thiz = NULL;
  bool isConstructing = JS_IsConstructing(cx, vp);
  if (isConstructing) {
    thiz = **instanceTemplate->NewInstance();
  } else {
    thiz = JS_THIS_OBJECT(cx, vp);
  }
  Arguments args(cx, thiz, argc, vp, data);
  Handle<Value> ret;
  if (callback) {
    ret = callback(args);
  }
  else {
    ret = v8::Undefined();
  }
  if (isConstructing) {
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(thiz));
  }
  else {
    JS_SET_RVAL(cx, vp, ret->native());
  }
  return !JS_IsExceptionPending(cx);
}

bool FunctionTemplate::IsFunctionTemplate(Handle<Value> v) {
  if (v.IsEmpty())
    return false;
  Handle<Object> o = v->ToObject();
  if (o.IsEmpty())
    return false;
  JSObject *obj = **o;
  return &sFunctionTemplateClass == JS_GET_CLASS(cx(), obj);
}

// static
Local<FunctionTemplate>
FunctionTemplate::New(InvocationCallback callback,
                      Handle<Value> data,
                      Handle<Signature>)
{
  FunctionTemplate ft;
  if (callback) {
    ft.SetCallHandler(callback, data);
  }
  return Local<FunctionTemplate>::New(&ft);
}

Local<Function>
FunctionTemplate::GetFunction()
{
  JSFunction* func =
    JS_NewFunction(cx(), CallCallback, 0, JSFUN_CONSTRUCTOR, NULL, NULL);
  JSObject* obj = JS_GetFunctionObject(func);
  Object o(obj);
  Local<String> prototypeStr = String::NewSymbol("prototype");
  (void)o.Set(prototypeStr, InternalObject().Get(prototypeStr));
  Local<Value> thiz = Local<Value>::New(&InternalObject());
  o.SetInternalField(0, thiz);
  return Local<Function>::New(reinterpret_cast<Function*>(&o));
}

void
FunctionTemplate::SetCallHandler(InvocationCallback callback,
                                 Handle<Value> data)
{
  JSIntPtr ptrcallback = reinterpret_cast<JSIntPtr>(callback);
  InternalObject().SetPointerInInternalField(kCallbackSlot, reinterpret_cast<void*>(ptrcallback & ~0x1));
  InternalObject().SetPointerInInternalField(kCallbackParitySlot, reinterpret_cast<void*>((ptrcallback & 0x1) << 1));

  if (data.IsEmpty()) {
    // XXX: this is not correct
    InternalObject().SetInternalField(kDataSlot, v8::Undefined());
  } else {
    InternalObject().SetInternalField(kDataSlot, data);
  }
}

Local<ObjectTemplate>
FunctionTemplate::InstanceTemplate()
{
  Local<Value> instance = InternalObject().GetInternalField(kInstanceSlot);
  if (instance.IsEmpty())
    return Local<ObjectTemplate>();
  return Local<ObjectTemplate>(reinterpret_cast<ObjectTemplate*>(*instance));
}

void
FunctionTemplate::Inherit(Handle<FunctionTemplate> parent)
{
  (void)JS_SetPrototype(cx(), InternalObject(), parent->InternalObject());
}

Local<ObjectTemplate>
FunctionTemplate::PrototypeTemplate()
{
  Local<Value> proto = InternalObject().Get(String::New("prototype"));
  if (proto.IsEmpty())
    return Local<ObjectTemplate>();
  return Local<ObjectTemplate>(reinterpret_cast<ObjectTemplate*>(*proto));
}

void
FunctionTemplate::SetClassName(Handle<String> name)
{
  UNIMPLEMENTEDAPI();
}

void
FunctionTemplate::SetHiddenPrototype(bool value)
{
  UNIMPLEMENTEDAPI();
}

bool
FunctionTemplate::HasInstance(Handle<Value> v)
{
  Handle<Object> object = v->ToObject();
  if (object.IsEmpty())
    return false;
  Handle<Object> proto = object->GetPrototype().As<Object>();
  if (proto.IsEmpty()) {
    return false;
  }
  return InternalObject().Equals(proto);
}

} // namespace v8
