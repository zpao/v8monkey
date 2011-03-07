#pragma once
// TODO: win32?
#define XP_UNIX
#include <jsapi.h>

namespace v8 {
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
  };

  class String : public Value  {
    JSString *mStr;
    String(JSString *s, size_t len);
  public:
    static String *New(const char *data);
    int Length() const;

    class AsciiValue {
    public:
      AsciiValue(Value val);
      ~AsciiValue();
      char* operator*() { return mStr; }
      const char* operator*() const { return mStr; }
      int length() const { return mLength; }
    private:
      char* mStr;
      int mLength;
    };

  private:
    int mLength;
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
  };
}
