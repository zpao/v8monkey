#include "v8-internal.h"

namespace v8 {
using namespace internal;

namespace {

JSClass gFunctionTemplateClass = {
  "FunctionTemplate", // name
  0, // flags
  JS_PropertyStub, // addProperty
  JS_PropertyStub, // delProperty
  JS_PropertyStub, // getProperty
  JS_StrictPropertyStub, // setProperty
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

} // anonymous namespace

FunctionTemplate::FunctionTemplate() :
  Template(&gFunctionTemplateClass)
{
}

// static
Local<FunctionTemplate>
FunctionTemplate::New(InvocationCallback callback,
                      Handle<Value> data,
                      Handle<Signature>)
{
  if (callback) {
    UNIMPLEMENTEDAPI(Local<FunctionTemplate>());
  }
  FunctionTemplate ft;
  return Local<FunctionTemplate>::New(&ft);
}

Local<Function>
FunctionTemplate::GetFunction()
{
  UNIMPLEMENTEDAPI(Local<Function>());
}

void
FunctionTemplate::SetCallHandler(InvocationCallback callback,
                                 Handle<Value> data)
{
  UNIMPLEMENTEDAPI();
}

Local<ObjectTemplate>
FunctionTemplate::InstanceTemplate()
{
  UNIMPLEMENTEDAPI(Local<ObjectTemplate>());
}

void
FunctionTemplate::Inherit(Handle<FunctionTemplate> parent)
{
  (void)JS_SetPrototype(cx(), InternalObject(), parent->InternalObject());
}

Local<ObjectTemplate>
FunctionTemplate::PrototypeTemplate()
{
  UNIMPLEMENTEDAPI(Local<ObjectTemplate>());
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
FunctionTemplate::HasInstance(Handle<Value> object)
{
  UNIMPLEMENTEDAPI(false);
}

} // namespace v8
