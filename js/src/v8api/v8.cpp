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

  JSObject *Context::getJSGlobal() {
    return mGlobal;
  }

  Context* Context::New() {
    return new Context;
  }

  // XXX: handle nested scopes - make this a stack
  static Context *gCurrentContext = 0;
  static Context *cx() {
    return gCurrentContext;
  }

  Context::Scope::Scope(Handle<Context> c) {
    gCurrentContext = *c;
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

  int String::Length() const {
    return JS_GetStringLength(*this);
  }

  int String::Utf8Length() const {
    return 0; // XXX Safe, but wrong
  }

  int String::Write(JSUint16* buffer,
                    int start,
                    int length,
                    WriteHints hints) const
  {
    return 0; // XXX Safe, but wrong
  }

  int String::WriteAscii(char* buffer,
                         int start,
                         int length,
                         WriteHints hints) const
  {
    size_t encodedLength = JS_GetStringEncodingLength(cx()->getJSContext(),
                                                      *this);
    char* tmp = new char[encodedLength];
    (void)WriteUtf8(tmp, encodedLength);

    // No easy way to convert UTF-8 to ASCII, so just drop characters that are
    // not ASCII.
    int toWrite = start + length;
    if (length == -1) {
      length = static_cast<int>(encodedLength);
      toWrite = length - start;
    }
    int idx = 0;
    for (int i = start; i < toWrite; i++) {
      if (static_cast<unsigned int>(tmp[i]) > 0x7F) {
        continue;
      }
      buffer[idx] = tmp[i];
      idx++;
    }

    delete[] tmp;
    return idx;
  }

  int String::WriteUtf8(char* buffer,
                        int length,
                        int* nchars_ref,
                        WriteHints hints) const
  {
    // TODO handle -1 for length!
    // XXX unclear if this includes the NULL terminator!
    size_t bytesWritten = JS_EncodeStringToBuffer(*this, buffer, length);
    // TODO check to make sure we don't overflow int
    if (nchars_ref) {
      *nchars_ref = static_cast<int>(bytesWritten);
    }
    return static_cast<int>(bytesWritten);
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
      int len = str->Length();
      mStr = new char[len];
      mLength = str->WriteAscii(mStr, 0, len);
    }
  }
  String::AsciiValue::~AsciiValue() {
    if (mStr)
      delete[] mStr;
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
    JSBool success = JS_ExecuteScript(cx()->getJSContext(), cx()->getJSGlobal(),
                                      mScript, &js_retval);
    Local<Value> retval = new Value(js_retval);

    return retval;
  }

  //////////////////////////////////////////////////////////////////////////////
  //// Handle class
  template <>
  Persistent<Value>::Persistent(Value *v) :
    Handle<Value>(v)
  {
    JS_AddValueRoot(cx()->getJSContext(), &v->native());
  }

  template <>
  void Persistent<Value>::Dispose() {
    JS_RemoveValueRoot(cx()->getJSContext(), &(*this)->native());
  }

  template <>
  Persistent<Context>::Persistent(Context *c) : Handle<Context>(c)
  {
  }

  template <>
  void Persistent<Context>::Dispose()
  {
  }
}
