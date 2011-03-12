#include <limits>
#include <algorithm>
#include "v8-internal.h"

namespace v8 {
  using namespace internal;

  //////////////////////////////////////////////////////////////////////////////
  //// Context class

  namespace internal {
    // XXX: handle nested scopes - make this a stack
    static Context *gCurrentContext = 0;
    Context *cx() {
      return gCurrentContext;
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
    gCurrentContext = this;
    JS_BeginRequest(mCtx);
  }

  void Context::Exit() {
    JS_EndRequest(mCtx);
    gCurrentContext = 0;
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

  Local<String> Value::ToString() const
  {
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

  //////////////////////////////////////////////////////////////////////////////
  //// Boolean class

  Handle<Boolean> Boolean::New(bool value) {
    static Boolean sTrue(true);
    static Boolean sFalse(false);
    return value ? &sTrue : &sFalse;
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
    Local<Script> script= new Script(s);
    return script;
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
