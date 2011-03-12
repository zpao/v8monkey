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

  String *String::New(const char *data,
                      int length)
  {
    if (length == -1) {
      length = strlen(data);
    }

    JSString *str = JS_NewStringCopyN(cx()->getJSContext(), data, length);
    return new String(str);
  }

  int String::Length() const {
    return JS_GetStringLength(*this);
  }

  int String::Utf8Length() const {
    size_t encodedLength = JS_GetStringEncodingLength(cx()->getJSContext(),
                                                      *this);
    return static_cast<int>(encodedLength);
  }

  int String::Write(JSUint16* buffer,
                    int start,
                    int length,
                    WriteHints hints) const
  {
    size_t internalLen;
    const jschar* chars =
      JS_GetStringCharsZAndLength(cx()->getJSContext(), *this, &internalLen);
    if (!chars || internalLen >= static_cast<size_t>(start)) {
      return 0;
    }
    size_t bytes = std::min<size_t>(length, internalLen - start) * 2;
    if (length == -1) {
      bytes = (internalLen - start) * 2;
    }
    (void)memcpy(buffer, &chars[start], bytes);

    return bytes / 2;
  }

  int String::WriteAscii(char* buffer,
                         int start,
                         int length,
                         WriteHints hints) const
  {
    size_t encodedLength = JS_GetStringEncodingLength(cx()->getJSContext(),
                                                      *this);
    char* tmp = new char[encodedLength];
    int written = WriteUtf8(tmp, encodedLength);

    // No easy way to convert UTF-8 to ASCII, so just drop characters that are
    // not ASCII.
    int toWrite = start + length;
    if (length == -1) {
      length = written;
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
    // If we have enough space for the NULL terminator, set it.
    if (idx <= length) {
      buffer[idx--] = '\0';
    }

    delete[] tmp;
    return idx - 1;
  }

  int String::WriteUtf8(char* buffer,
                        int length,
                        int* nchars_ref,
                        WriteHints hints) const
  {
    // TODO handle -1 for length!
    // XXX unclear if this includes the NULL terminator!
    size_t bytesWritten = JS_EncodeStringToBuffer(*this, buffer, length);

    // If we have enough space for the NULL terminator, set it.
    if (bytesWritten < length) {
      buffer[bytesWritten + 1] = '\0';
    }

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
      // Need space for the NULL terminator.
      mStr = new char[len + 1];
      mLength = str->WriteAscii(mStr, 0, len + 1);
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
    (void)JS_ExecuteScript(cx()->getJSContext(), cx()->getJSGlobal(),
                           mScript, &js_retval);
    // js_retval will be unchanged on failure, so it's a JSVAL_VOID.
    Local<Value> retval = new Value(js_retval);

    return retval;
  }
}
