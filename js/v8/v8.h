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

  class String {
    JSString *mStr;
    String(JSString *s);
  public:
    static String *New(const char *data);
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
}
