#pragma once
#include <jsapi.h>

namespace v8 {
  // Define some classes first so we can use them before fully defined
  class String;
  class Context;
  template <class T> class Handle;
  template <class T> class Local;
  template <class T> class Persistent;

  namespace internal {
    class GCOps;
    class GCReferenceContainer;
    class RCOps;
    class RCReferenceContainer;

    class GCReference {
      void root(JSContext *ctx) {
        JS_AddValueRoot(ctx, &mVal);
      }
      void unroot(JSContext *ctx) {
        JS_RemoveValueRoot(ctx, &mVal);
      }
      GCReference *Globalize();
      void Dispose();

    protected:
      jsval mVal;

      friend class GCOps;
      template <class T> friend class v8::Local;
      template <class T> friend class v8::Persistent;

      template <typename T>
      T* As() {
        return reinterpret_cast<T*>(this);
      }
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
    };

    class RCReference {
      size_t mRefCount;

      friend class RCOps;
      template <class T> friend class v8::Local;
      template <class T> friend class v8::Persistent;

      void addRef() {
        mRefCount++;
      }
      void release() {
        if (0 == --mRefCount) {
          delete this;
        }
      }
    public:
      RCReference() : mRefCount(0)
      {}
    };

    void FinalizeContext(JSContext *ctx, JSObject *obj);
  }

  struct HandleScope {
    HandleScope();
    ~HandleScope();
  private:
    template <class T> friend class Local;
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
  protected:
    internal::GCReference *asGCRef() {
      return reinterpret_cast<internal::GCReference*>(mVal);
    }
  public:
    Handle() : mVal(NULL) {}
    Handle(T *val) : mVal(val) {}

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
      internal::GCReference *ref = HandleScope::CreateHandle(other.asGCRef());
      return Local<T>(ref->As<T>());
    }
  };

  template <typename T>
  class Persistent : public Handle<T> {
  public:
    Persistent() : Handle<T>() {}

    template <class S>
    Persistent(S *val) : Handle<T>(val) {}

    void Dispose() {
      internal::GCReference *ref = this->asGCRef();
      ref->Dispose();
    }

    static Persistent<T> New(Handle<T> other) {
      internal::GCReference *ref = other.asGCRef()->Globalize();
      return Persistent<T>(ref->As<T>());
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
    uint32_t* stack_limit() const { return mStackLimit; }
    void set_stack_limit(uint32_t* value) { mStackLimit = value; }
  private:
    int mMaxYoungSpaceSize;
    int mMaxOldSpaceSize;
    int mMaxExecutableSize;
    uint32_t* mStackLimit;
  };
  bool SetResourceConstraints(ResourceConstraints *constraints);

  // Parent class of every JS val / object
  class Value : protected internal::GCReference {
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
    Local<String> ToString() const;

    jsval &native() { return mVal; }
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

    static String *New(const char *data, int length = -1);
  };

  class Script {
    JSScript *mScript;
    Script(JSScript *s);
  public:
    // TODO follow the v8 api
    static Local<Script> Compile(Handle<String> str);

    Local<Value> Run();
  };

  template <>
  inline Local<Context> Local<Context>::New(Handle<Context> other) {
    if (other.IsEmpty())
      return Local<Context>();
    other->addRef();
    return Local<Context>(*other);
  }

  template <>
  inline void Persistent<Context>::Dispose() {
    (*this)->release();
  }

  template <>
  inline Persistent<Context> Persistent<Context>::New(Handle<Context> other) {
    if (other.IsEmpty())
      return Persistent<Context>();
    other->addRef();
    return Persistent<Context>(*other);
  }

}
