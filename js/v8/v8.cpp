#include <limits>
#include "v8.h"

namespace v8 {
  namespace {
    const int KB = 1024;
    const int MB = 1024 * 1024;
    JSRuntime *gRuntime = 0;
    JSRuntime *rt() {
      if (!gRuntime) {
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
    // TODO implement me!
    return Local<String>();
  }

  //////////////////////////////////////////////////////////////////////////////
  //// String class

  String::String(JSString *s, size_t len) :
    mStr(s),
    mLength(len)
  {
    // TODO: handle this some other way.
    if (len > std::numeric_limits<int>::max()) {
      exit (1);
    }
  }

  String *String::New(const char *data) {
    JSString *str = JS_NewStringCopyZ(cx()->getJSContext(), data);
    return new String(str, strlen(data));
  }

  int String::Length() const {
    return mLength;
  }

  JSString *String::getJSString() {
    return mStr;
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
      JS_EncodeStringToBuffer(str->getJSString(), mStr, mLength);
    }
  }
  String::AsciiValue::~AsciiValue() {
    delete mStr;
  }
}
