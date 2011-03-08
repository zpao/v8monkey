#include <limits>
#include "v8.h"

namespace v8 {
  namespace {
    const int KB = 1024;
    const int MB = 1024 * 1024;
    JSRuntime *gRuntime = 0;
    JSRuntime *rt() {
      if (!gRuntime) {
        JS_CStringsAreUTF8();
        gRuntime = JS_NewRuntime(64 * MB);
      }
      return gRuntime;
    }

    // TODO: call this
    void shutdown() {
      if (gRuntime)
        JS_DestroyRuntime(gRuntime);
      JS_ShutDown();
    }

    JSClass global_class = {
     "global", JSCLASS_GLOBAL_FLAGS,
     JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
     JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
     JSCLASS_NO_OPTIONAL_MEMBERS
    };

    void reportError(JSContext *ctx, const char *message, JSErrorReport *report) {
      fprintf(stderr, "%s:%u:%s\n",
              report->filename ? report->filename : "<no filename>",
              (unsigned int) report->lineno,
              message);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  //// Context class

  Context::Context()
    : mCtx(JS_NewContext(rt(), 8192))
  {
    JS_SetOptions(mCtx, JSOPTION_VAROBJFIX | JSOPTION_JIT | JSOPTION_METHODJIT);
    JS_SetVersion(mCtx, JSVERSION_LATEST);
    JS_SetErrorReporter(mCtx, reportError);
    mGlobal = JS_NewCompartmentAndGlobalObject(mCtx, &global_class, NULL);

    JS_InitStandardClasses(mCtx, mGlobal);
  }

  Context::~Context() {
    JS_DestroyContext(mCtx);
  }

  JSContext *Context::getJSContext() {
    return mCtx;
  }

  Context* Context::New() {
    return new Context;
  }

  // XXX: handle nested scopes - make this a stack
  static Context *gCurrentContext = 0;
  static Context *cx() {
    return gCurrentContext;
  }

  Context::Scope::Scope(Context *c) {
    gCurrentContext = c;
  }

  Context::Scope::~Scope() {
    gCurrentContext = 0;
  }

  //////////////////////////////////////////////////////////////////////////////
  //// Value class

  Local<String> Value::ToString() const
  {
    // TODO Allocate this in a way that doesn't leak
    JSString *str(JS_ValueToString(cx()->getJSContext(), mVal));
    Local<String> s = new String(str);
    return s;
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
  //// String class

  String::String(JSString *s) {
    mVal = STRING_TO_JSVAL(s);
  }

  String *String::New(const char *data) {
    JSString *str = JS_NewStringCopyZ(cx()->getJSContext(), data);
    return new String(str);
  }

  JSString *String::asJSString() const {
    return JSVAL_TO_STRING(mVal);
  }

  int String::Length() const {
    return JS_GetStringLength(asJSString());
  }

  int String::Utf8Length() const {
    return 0; // Safe, but wrong
  }

  String::AsciiValue::AsciiValue(Handle<v8::Value> val) {
    // Set some defaults which will be used for empty values/strings
    mStr = NULL;
    mLength = 0;

    if (val.IsEmpty()) {
      return;
    }

    // TODO: Do we need something about HandleScope here?
    Local<String> str = val->ToString();
    if (!str.IsEmpty()) {
      mLength = (size_t)str->Length();
      // FIXME sdwilsh
      JS_EncodeStringToBuffer(NULL, mStr, mLength);
    }
  }
  String::AsciiValue::~AsciiValue() {
    delete mStr;
  }

  //////////////////////////////////////////////////////////////////////////////
  //// Handle class
  template <>
  Persistent<Value>::Persistent(Value *v) :
    Handle<Value>(v)
  {
    JS_AddValueRoot(cx()->getJSContext(), &v->getJsval());
  }

  template <>
  void Persistent<Value>::Dispose() {
    JS_RemoveValueRoot(cx()->getJSContext(), &(*this)->getJsval());
  }
}
