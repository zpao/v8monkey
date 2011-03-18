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
    JSCrossCompartmentCall *call;
    ContextChain *next;
  };
  static ContextChain *gContextChain = 0;
}

Context::Context(JSObject *global) :
  internal::GCReference(OBJECT_TO_JSVAL(global))
{}

Local<Context> Context::GetEntered() {
  return Local<Context>::New(gContextChain->ctx);
}

Local<Context> Context::GetCurrent() {
  // XXX: This is probably not right
  return Local<Context>::New(gContextChain->ctx);
}

void Context::Enter() {
  ContextChain *link = new ContextChain;
  link->next = gContextChain;
  link->ctx = this;
  link->call = JS_EnterCrossCompartmentCall(cx(), *this);
  gContextChain = link;
}

void Context::Exit() {
  JS_LeaveCrossCompartmentCall(gContextChain->call);
  ContextChain *link = gContextChain;
  gContextChain = gContextChain->next;
  delete link;
}

Persistent<Context> Context::New() {
  JSObject *global = JS_NewCompartmentAndGlobalObject(cx(), &global_class, NULL);

  JSCrossCompartmentCall *call = JS_EnterCrossCompartmentCall(cx(), global);
  JS_InitStandardClasses(cx(), global);
  JS_LeaveCrossCompartmentCall(call);

  return Persistent<Context>(new Context(global));
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
  if (!JS_ValueToBoolean(cx(), mVal, &b))
    return Local<Boolean>();

  Boolean value(b);
  return Local<Boolean>::New(&value);
}

Local<Number> Value::ToNumber() const {
  double d;
  if (JS_ValueToNumber(cx(), mVal, &d))
    return Local<Number>();
  return Number::New(d);
}

Local<String> Value::ToString() const {
  // TODO Allocate this in a way that doesn't leak
  JSString *str(JS_ValueToString(cx(), mVal));
  String s(str);
  return Local<String>::New(&s);
}

bool Value::IsFunction() const {
  if (!IsObject())
    return false;
  JSObject *obj = JSVAL_TO_OBJECT(mVal);
  return JS_ObjectIsFunction(cx(), obj);
}

bool Value::IsArray() const {
  if (!IsObject())
    return false;
  JSObject *obj = JSVAL_TO_OBJECT(mVal);
  return JS_IsArrayObject(cx(), obj);
}

bool Value::IsDate() const {
  if (!IsObject())
    return false;
  JSObject *obj = JSVAL_TO_OBJECT(mVal);
  return JS_ObjectIsDate(cx(), obj);
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
  (void) JS_StrictlyEqual(cx(), mVal, other->native(), &equal);
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
  if (!JS_NewNumberValue(cx(), d, &val))
    return Local<Number>();

  Number n(val);
  return Local<Number>::New(&n);
}


//////////////////////////////////////////////////////////////////////////////
//// Integer class

Local<Integer> Integer::New(JSInt32 value) {
  jsval val = INT_TO_JSVAL(value);
  Integer i(val);
  return Local<Integer>::New(&i);
}
Local<Integer> Integer::NewFromUnsigned(JSUint32 value) {
  jsval val = UINT_TO_JSVAL(value);
  Integer i(val);
  return Local<Integer>::New(&i);
}
JSInt64 Integer::Value() const {
  // XXX We should keep track of mIsDouble or something, but that wasn't working...
  if (JSVAL_IS_INT(mVal)) {
    return static_cast<JSInt64>(JSVAL_TO_INT(mVal));
  }
  else {
    return static_cast<JSInt64>(JSVAL_TO_DOUBLE(mVal));
  }
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
Script::Script(JSScript *s)
{
  JSObject *obj = JS_NewScriptObject(cx(), s);
  JS_SetPrivate(cx(), obj, s);
  mVal = OBJECT_TO_JSVAL(obj);
}

Script::operator JSScript *() {
  return reinterpret_cast<JSScript*>(JS_GetPrivate(cx(), JSVAL_TO_OBJECT(mVal)));
}

Local<Script> Script::Compile(Handle<String> source) {
  JS::Anchor<JSString*> anchor(JSVAL_TO_STRING(source->native()));
  const jschar* chars;
  size_t len;
  chars = JS_GetStringCharsAndLength(cx(),
                                     anchor.get(), &len);

  JSScript* s = JS_CompileUCScript(cx(), **Context::GetCurrent()->Global(),
                                   chars, len, NULL, NULL);
  return Local<Script>::New(new Script(s));
}

Local<Value> Script::Run() {
  jsval js_retval;
  (void)JS_ExecuteScript(cx(), **Context::GetCurrent()->Global(),
                         *this, &js_retval);
  // js_retval will be unchanged on failure, so it's a JSVAL_VOID.
  Value v(js_retval);
  return Local<Value>::New(&v);
}
}
