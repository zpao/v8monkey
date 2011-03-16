#include <limits>
#include <algorithm>
#include "v8-internal.h"

namespace v8 {
using namespace internal;

//////////////////////////////////////////////////////////////////////////////
//// Context class

namespace internal {
  struct ContextChain {
    Context* ctx;
    ContextChain *next;
  };
  static ContextChain *gContextChain = 0;
  Context *cx() {
    return gContextChain->ctx;
  }

  static Persistent<Context> gRootContext;
  Persistent<Context> gcx() {
    if (gRootContext.IsEmpty()) {
      gRootContext = Context::New();
    }
    return gRootContext;
  }
}

Context::Context(JSContext *ctx, JSObject *global) :
  mCtx(ctx), mGlobal(global)
{}

JSContext *Context::getJSContext() {
  return mCtx;
}

JSObject *Context::getJSGlobal() {
  return mGlobal;
}

void Context::Enter() {
  ContextChain *link = new ContextChain;
  link->next = gContextChain;
  link->ctx = this;
  gContextChain = link;
  JS_BeginRequest(mCtx);
}

void Context::Exit() {
  JS_EndRequest(mCtx);
  ContextChain *link = gContextChain;
  gContextChain = gContextChain->next;
  delete link;
}

Persistent<Context> Context::New() {
  JSContext *ctx(JS_NewContext(rt(), 8192));
  JS_SetOptions(ctx, JSOPTION_VAROBJFIX | JSOPTION_JIT | JSOPTION_METHODJIT);
  JS_SetVersion(ctx, JSVERSION_LATEST);
  JS_SetErrorReporter(ctx, reportError);

  JS_BeginRequest(ctx);
  JSObject *global = JS_NewCompartmentAndGlobalObject(ctx, &global_class, NULL);

  JS_InitStandardClasses(ctx, global);

  JS_EndRequest(ctx);

  return Persistent<Context>(new Context(ctx, global));
}

//////////////////////////////////////////////////////////////////////////////
//// Resource Constraints

ResourceConstraints::ResourceConstraints() :
  mMaxYoungSpaceSize(0),
  mMaxOldSpaceSize(0),
  mMaxExecutableSize(0),
  mStackLimit(NULL)
{
}

bool SetResourceConstraints(ResourceConstraints *constraints) {
  // TODO: Pull the relevent information out that applies to SM and set some
  //       globals that would be used.
  return true;
}

//////////////////////////////////////////////////////////////////////////////
//// Value class

Local<Boolean> Value::ToBoolean() const {
  JSBool b;
  if (!JS_ValueToBoolean(*cx(), mVal, &b))
    return Local<Boolean>();

  Boolean value(b);
  return Local<Boolean>::New(&value);
}

Local<Number> Value::ToNumber() const {
  double d;
  if (JS_ValueToNumber(*cx(), mVal, &d))
    return Local<Number>();
  return Number::New(d);
}

Local<String> Value::ToString() const {
  // TODO Allocate this in a way that doesn't leak
  JSString *str(JS_ValueToString(cx()->getJSContext(), mVal));
  String s(str);
  return Local<String>::New(&s);
}

bool Value::IsFunction() const {
  if (!IsObject())
    return false;
  JSObject *obj = JSVAL_TO_OBJECT(mVal);
  return JS_ObjectIsFunction(cx()->getJSContext(), obj);
}

bool Value::IsArray() const {
  if (!IsObject())
    return false;
  JSObject *obj = JSVAL_TO_OBJECT(mVal);
  return JS_IsArrayObject(cx()->getJSContext(), obj);
}

bool Value::IsDate() const {
  if (!IsObject())
    return false;
  JSObject *obj = JSVAL_TO_OBJECT(mVal);
  return JS_ObjectIsDate(cx()->getJSContext(), obj);
}

bool
Value::BooleanValue() const
{
  return IsTrue();
}

double
Value::NumberValue() const
{
  UNIMPLEMENTEDAPI(0);
}

JSInt64
Value::IntegerValue() const
{
  UNIMPLEMENTEDAPI(0);
}

JSUint32
Value::Uint32Value() const
{
  UNIMPLEMENTEDAPI(0);
}

JSInt32
Value::Int32Value() const
{
  UNIMPLEMENTEDAPI(0);
}

bool
Value::Equals(Handle<Value> other) const
{
  UNIMPLEMENTEDAPI(false);
}

bool
Value::StrictEquals(Handle<Value> other) const
{
  JSBool equal;
  // XXX: check for error. This can fail if they are ropes that fail to turn into strings
  (void) JS_StrictlyEqual(*cx(), mVal, other->native(), &equal);
  return equal == JS_TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//// Boolean class

Handle<Boolean> Boolean::New(bool value) {
  static Boolean sTrue(true);
  static Boolean sFalse(false);
  return value ? &sTrue : &sFalse;
}

//////////////////////////////////////////////////////////////////////////////
//// Number class

Local<Number> Number::New(double d) {
  jsval val;
  if (!JS_NewNumberValue(*cx(), d, &val))
    return Local<Number>();

  Number n(val);
  return Local<Number>::New(&n);
}


//////////////////////////////////////////////////////////////////////////////
//// Primitives & basic values

Handle<Primitive> Undefined() {
  UNIMPLEMENTEDAPI(Handle<Primitive>());
}

Handle<Primitive> Null() {
  UNIMPLEMENTEDAPI(Handle<Primitive>());
}

Handle<Boolean> True() {
  UNIMPLEMENTEDAPI(Handle<Boolean>());
}

Handle<Boolean> False() {
  UNIMPLEMENTEDAPI(Handle<Boolean>());
}

//////////////////////////////////////////////////////////////////////////////
//// Script class
Script::Script(JSScript *s) :
  mScript(s)
{
}

Local<Script> Script::Compile(Handle<String> source) {
  // TODO: we might need to do something to prevent GC?
  const jschar* chars;
  size_t len;
  chars = JS_GetStringCharsAndLength(cx()->getJSContext(),
                                     JSVAL_TO_STRING(source->native()), &len);

  JSScript* s = JS_CompileUCScript(cx()->getJSContext(), cx()->getJSGlobal(),
                                   chars, len, NULL, NULL);
  return Local<Script>::New(new Script(s));
}

Local<Value> Script::Run() {
  jsval js_retval;
  (void)JS_ExecuteScript(cx()->getJSContext(), cx()->getJSGlobal(),
                         mScript, &js_retval);
  // js_retval will be unchanged on failure, so it's a JSVAL_VOID.
  Value v(js_retval);
  return Local<Value>::New(&v);
}
}
