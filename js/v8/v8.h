#pragma once
// TODO: win32?
#define XP_UNIX
#include <jsapi.h>

namespace v8 {
  // Define some classes first so we can use them before fully defined
  class String;
  template <class T> class Handle;
  template <class T> class Local;

  struct HandleScope {
  };

  class Context {
    JSContext *mCtx;
    JSObject *mGlobal;

    Context();
    ~Context();

  public:
    static Context* New();

    JSContext *getJSContext();

    struct Scope {
      Scope(Context *ctx);
      ~Scope();
    };
  };

  // Parent class of every JS val / object
  class Value {
  protected:
    jsval mVal;
  public:
    bool IsUndefined() const { return JSVAL_IS_VOID(mVal); }
    bool IsNull() const { return JSVAL_IS_NULL(mVal); }
    bool IsTrue() const { return IsBoolean() && JSVAL_TO_BOOLEAN(mVal); }
    bool IsFalse() const { return IsBoolean() && !JSVAL_TO_BOOLEAN(mVal); }

    bool IsString() const { return JSVAL_IS_STRING(mVal); }
    bool IsFunction() const;
    bool IsArray() const;
    bool IsObject() const { return !JSVAL_IS_PRIMITIVE(mVal); }
    bool IsBoolean() const { return JSVAL_IS_BOOLEAN(mVal); }
    bool IsNumber() const { return JSVAL_IS_NUMBER(mVal); }
    bool IsInt32() const { return JSVAL_IS_INT(mVal); }
    bool IsDate() const;
    Local<String> ToString() const;

    jsval &getJsval() { return mVal; }
  };

  class Boolean : public Value {
    Boolean(JSBool val) {
      mVal = val ? JSVAL_TRUE : JSVAL_FALSE;
    }
  public:
    bool Value() const {
      return mVal == JSVAL_TRUE;
    }
    static Handle<Boolean> New(bool value);
  };

  class String : public Value  {
    String(JSString *s);

    JSString *asJSString() const;
  public:
    static String *New(const char *data);
    int Length() const;
    int Utf8Length() const;

    class AsciiValue {
      char* mStr;
      int mLength;
    public:
      AsciiValue(Handle<v8::Value> val);
      ~AsciiValue();
      char* operator*() { return mStr; }
      const char* operator*() const { return mStr; }
      int length() const { return mLength; }
    };
  };

  template <typename T>
  class Handle {
    T* mVal;
  public:
    Handle() : mVal(NULL) {}
    Handle(T *val) : mVal(val) {}

    bool IsEmpty() const {
      return mVal != 0;
    }
    T* operator ->() {
      return mVal;
    }
    T* operator *() {
      return mVal;
    }
  };

  template <typename T>
  class Local : public Handle<T> {
  public:
    Local() : Handle<T>() {}
    Local(T *val) : Handle<T>(val) {}
  };

  template <typename T>
  class Persistent : public Handle<T> {
  public:
    Persistent() : Handle<T>() {}
    Persistent(T *val);

    void Dispose();
  };
}
