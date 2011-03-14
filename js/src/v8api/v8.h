#pragma once
#include <jsapi.h>

namespace v8 {
// Define some classes first so we can use them before fully defined
class HandleScope;
class Boolean;
class Number;
class String;
class Context;
template <class T> class Handle;
template <class T> class Local;
template <class T> class Persistent;

namespace internal {
  class GCReference;
  class GCOps;
  class GCReferenceContainer;
  class RCOps;
  class RCReferenceContainer;
}

namespace internal {
  class GCReference {
    friend class GCOps;

    void root(JSContext *ctx) {
      JS_AddValueRoot(ctx, &mVal);
    }
    void unroot(JSContext *ctx) {
      JS_RemoveValueRoot(ctx, &mVal);
    }

  protected:
    jsval mVal;
  public:
    GCReference(jsval val) :
      mVal(val)
    {}
    GCReference() :
      mVal(JSVAL_VOID)
    {}
    jsval &native() {
      return mVal;
    }

    GCReference *Globalize();
    void Dispose();
    GCReference *Localize();
  };

  class RCReference {
    size_t mRefCount;

  public:
    RCReference() : mRefCount(0)
    {}

    RCReference *Globalize() {
      mRefCount++;
      return this;
    }

    void Dispose() {
      if (0 == --mRefCount) {
        delete this;
      }
    }

    RCReference *Localize() {
      return Globalize();
    }
  };
}

struct HandleScope {
  HandleScope();
  ~HandleScope();
private:
  friend class internal::GCReference;
  static internal::GCReference *CreateHandle(internal::GCReference r);

  static HandleScope *sCurrent;
  size_t getHandleCount();

  internal::GCReferenceContainer *mGCReferences;
  internal::RCReferenceContainer *mRCReferences;
  HandleScope *mPrevious;
};

template <typename T>
class Handle {
  T* mVal;
public:
  Handle() : mVal(NULL) {}
  Handle(T *val) : mVal(val) {}

  template <typename S>
  Handle(Handle<S> h) : mVal(*h) {}

  bool IsEmpty() const {
    return mVal == 0;
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

  static inline Local<T> New(Handle<T> other) {
    if (other.IsEmpty())
      return Local<T>();
    return reinterpret_cast<T*>(other->Localize());
  }
};

template <typename T>
class Persistent : public Handle<T> {
public:
  Persistent() : Handle<T>() {}

  template <class S>
  Persistent(S *val) : Handle<T>(val) {}

  void Dispose() {
    (*this)->Dispose();
  }

  static Persistent<T> New(Handle<T> other) {
    if (other.IsEmpty())
      return Persistent<T>();
    return reinterpret_cast<T*>(other->Globalize());
  }
};

class Context : public internal::RCReference {
  JSContext *mCtx;
  JSObject *mGlobal;

  Context(JSContext *ctx, JSObject *global);
public:
  void Enter();
  void Exit();

  static Persistent<Context> New();

  JSContext *getJSContext();

  // TODO: expose Local<Object> Global instead
  JSObject *getJSGlobal();

  struct Scope {
    Scope(Handle<Context> ctx) :
      mCtx(ctx) {
      mCtx->Enter();
    }
    ~Scope() {
      mCtx->Exit();
    }
  private:
    Handle<Context> mCtx;
  };

  operator JSContext*() const { return mCtx; }
};

class ResourceConstraints {
public:
  ResourceConstraints();
  int max_young_space_size() const { return mMaxYoungSpaceSize; }
  void set_max_young_space_size(int value) { mMaxYoungSpaceSize = value; }
  int max_old_space_size() const { return mMaxOldSpaceSize; }
  void set_max_old_space_size(int value) { mMaxOldSpaceSize = value; }
  int max_executable_size() { return mMaxExecutableSize; }
  void set_max_executable_size(int value) { mMaxExecutableSize = value; }
  JSUint32* stack_limit() const { return mStackLimit; }
  void set_stack_limit(JSUint32* value) { mStackLimit = value; }
private:
  int mMaxYoungSpaceSize;
  int mMaxOldSpaceSize;
  int mMaxExecutableSize;
  JSUint32* mStackLimit;
};
bool SetResourceConstraints(ResourceConstraints *constraints);

// Parent class of every JS val / object
class Value : public internal::GCReference {
public:
  Value() : internal::GCReference()
  {}
  Value(jsval val) : internal::GCReference(val)
  {}
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

  Local<Boolean> ToBoolean() const;
  Local<Number> ToNumber() const;
  Local<String> ToString() const;

  bool StrictEquals(Handle<Value> other);
};

class Boolean : public Value {
public:
  Boolean(JSBool val) {
    mVal = val ? JSVAL_TRUE : JSVAL_FALSE;
  }
  bool Value() const {
    return mVal == JSVAL_TRUE;
  }
  static Handle<Boolean> New(bool value);
};

class Number : public Value {
  Number(double v) {
    mVal = v;
  }
public:
  inline double Value() const {
    return JSVAL_TO_DOUBLE(mVal);
  }
  static Local<Number> New(double value);
};

class String : public Value  {
  friend class Value;

  String(JSString *s);

  operator JSString*() const { return JSVAL_TO_STRING(mVal); }
public:
  int Length() const;
  int Utf8Length() const;

  enum WriteHints {
    NO_HINTS = 0,
    HINT_MANY_WRITES_EXPECTED = 1
  };
  int Write(JSUint16* buffer,
            int start = 0,
            int length = -1,
            WriteHints hints = NO_HINTS) const;
 int WriteAscii(char* buffer,
                int start = 0,
                int length = -1,
                WriteHints hints = NO_HINTS) const;
 int WriteUtf8(char* buffer,
               int length= -1,
               int* nchars_ref = NULL,
               WriteHints hints = NO_HINTS) const;

  class AsciiValue {
    char* mStr;
    int mLength;
  public:
    explicit AsciiValue(Handle<v8::Value> val);
    ~AsciiValue();
    char* operator*() { return mStr; }
    const char* operator*() const { return mStr; }
    int length() const { return mLength; }
  };

  static Local<String> New(const char *data, int length = -1);
};

enum PropertyAttribute {
  None = 0,
  ReadOnly = 1 << 0,
  DontEnum = 1 << 1,
  DontDelete = 1 << 2
};

class Object : public Value {
  Object(JSObject *obj);
  operator JSObject *() const { return JSVAL_TO_OBJECT(mVal); }
public:
  bool Set(Handle<Value> key, Handle<Value> value, PropertyAttribute attribs = None);
  Local<Value> Get(Handle<Value> key);

  static Local<Object> New();
};

class Script : public internal::RCReference {
  JSScript *mScript;
  Script(JSScript *s);
public:
  // TODO follow the v8 api
  static Local<Script> Compile(Handle<String> str);

  Local<Value> Run();
};
}
