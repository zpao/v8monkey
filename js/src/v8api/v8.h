#pragma once
#include <jsapi.h>

namespace v8 {
// Define some classes first so we can use them before fully defined
struct HandleScope;
class Boolean;
class Number;
class Integer;
class String;
class Array;
class Context;
class Function;
class AccessorInfo;
class ObjectTemplate;
class Signature;
template <class T> class Handle;
template <class T> class Local;
template <class T> class Persistent;

#define UNIMPLEMENTEDAPI(rval) do { v8::internal::notImplemented(); return rval; } while(0)

namespace internal {
  class GCReference;
  class GCOps;
  class GCReferenceContainer;
  class RCOps;
  class RCReferenceContainer;
}

namespace internal {
  void notImplemented();

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

class Data : public internal::GCReference {
public:
  Data() : internal::GCReference() { }
  Data(jsval val) : internal::GCReference(val) { }
};

// Parent class of every JS val / object
class Value : public Data {
public:
  Value() : Data() { }
  Value(jsval val) : Data(val) { }
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

class Primitive : public Value { };

class Boolean : public Primitive {
public:
  Boolean(JSBool val) {
    mVal = val ? JSVAL_TRUE : JSVAL_FALSE;
  }
  bool Value() const {
    return mVal == JSVAL_TRUE;
  }
  static Handle<Boolean> New(bool value);
};

class Number : public Primitive {
  Number(jsval v) {
    mVal = v;
  }
public:
  inline double Value() const {
    return JSVAL_TO_DOUBLE(mVal);
  }
  static Local<Number> New(double value);
};

class String : public Primitive  {
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


Handle<Primitive> Undefined();
Handle<Primitive> Null();
Handle<Boolean> True();
Handle<Boolean> False();

enum PropertyAttribute {
  None = 0,
  ReadOnly = 1 << 0,
  DontEnum = 1 << 1,
  DontDelete = 1 << 2
};

typedef Handle<Value> (*AccessorGetter)(Local<String> property, const AccessorInfo &info);
typedef Handle<Value> (*AccessorSetter)(Local<String> property, Local<Value> value, const AccessorInfo &info);

enum AccessControl {
  DEFAULT = 0,
  ALL_CAN_READ = 1,
  ALL_CAN_WRITE = 1 << 1,
  PROHIBITS_OVERWRITING = 1 << 2
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

class Template {
public:
  void Set(Handle<String> name, Handle<Value> data, PropertyAttribute attribs = None) {
    UNIMPLEMENTEDAPI();
  }

  template <typename T>
  void Set(const char *name, Handle<T> data) {
    Set(String::New(name), data);
  }
private:
  Template();

  friend class FunctionTemplate;
  friend class ObjectTemplate;
};

class Arguments {
public:
  int Length() const {
    UNIMPLEMENTEDAPI(0);
  }
  Local<Value> operator[](int i) const {
    UNIMPLEMENTEDAPI(NULL);
  }
  Local<Function> Callee() const {
    UNIMPLEMENTEDAPI(NULL);
  }
  Local<Object> This() const {
    UNIMPLEMENTEDAPI(NULL);
  }
  Local<Object> Holder() const {
    UNIMPLEMENTEDAPI(NULL);
  }
  bool IsConstructCall() const {
    UNIMPLEMENTEDAPI(false);
  }
  Local<Value> Data() const {
    UNIMPLEMENTEDAPI(NULL);
  }
};

class AccessorInfo {
  Local<Value> Data() const {
    UNIMPLEMENTEDAPI(NULL);
  }
  Local<Object> This() const {
    UNIMPLEMENTEDAPI(NULL);
  }
  Local<Object> Holder() const {
    UNIMPLEMENTEDAPI(NULL);
  }
};

typedef Handle<Value> (*InvocationCallback)(const Arguments &args);
typedef Handle<Value> (*NamedPropertyGetter)(Local<String> property, const AccessorInfo &info);
typedef Handle<Value> (*NamedPropertySetter)(Local<String> property, Local<Value> value, const AccessorInfo &info);
typedef Handle<Integer> (*NamedPropertyQuery)(Local<String> property, const AccessorInfo &info);
typedef Handle<Boolean> (*NamedPropertyDeleter)(Local<String> property, const AccessorInfo &info);
typedef Handle<Array> (*NamedPropertyEnumerator)(const AccessorInfo &info);

typedef Handle<Value> (*IndexedPropertyGetter)(uint32_t index, const AccessorInfo &info);
typedef Handle<Value> (*IndexedPropertySetter)(uint32_t index, Local<Value> value, const AccessorInfo &info);
typedef Handle<Integer> (*IndexedPropertyQuery)(uint32_t index, const AccessorInfo &info);
typedef Handle<Boolean> (*IndexedPropertyDeleter)(uint32_t index, AccessorInfo &info);
typedef Handle<Array> (*IndexedPropertyEnumerator)(const AccessorInfo &info);

enum AccessType {
  ACCESS_GET,
  ACCESS_SET,
  ACCESS_HAS,
  ACCESS_DELETE,
  ACCESS_KEYS
};

typedef bool (*NamedSecurityCallback)(Local<Object> host, Local<Value> key, AccessType type, Local<Value> data);
typedef bool (*IndexedSecurityCallback)(Local<Object> host, uint32_t index, AccessType type, Local<Value> data);

class FunctionTemplate : public Template {
  static Local<FunctionTemplate> New(InvocationCallback callback, Handle<Value> data = Handle<Value>(), Handle<Signature> signature = Handle<Signature>());
  Local<Function> GetFunction () {
    UNIMPLEMENTEDAPI(NULL);
  }

  void SetCallHandler(InvocationCallback callback, Handle<Value> data = Handle<Value>()) {
    UNIMPLEMENTEDAPI();
  }

  Local<ObjectTemplate> InstanceTemplate() {
    UNIMPLEMENTEDAPI(NULL);
  }

  void Inherit(Handle<FunctionTemplate> parent) {
    UNIMPLEMENTEDAPI();
  }

  Local<ObjectTemplate> PrototypeTemplate() {
    UNIMPLEMENTEDAPI(NULL);
  }

  void SetClassName(Handle<String> name) {
    UNIMPLEMENTEDAPI();
  }

  void SetHiddenPrototype(bool value) {
    UNIMPLEMENTEDAPI();
  }

  bool HasInstance(Handle<Value> object) {
    UNIMPLEMENTEDAPI(false);
  }
};

class ObjectTemplate : public Template {
  static Local<ObjectTemplate> New() {
    UNIMPLEMENTEDAPI(NULL);
  }

  Local<Object> NewInstance() {
    UNIMPLEMENTEDAPI(NULL);
  }

  void SetAccessor(Handle<String> name, AccessorGetter getter, AccessorSetter setter, Handle<Value> data = Handle<Value>(), AccessControl settings = DEFAULT, PropertyAttribute attribs = None) {
    UNIMPLEMENTEDAPI();
  }

  void SetNamedPropertyHandler(NamedPropertyGetter getter, NamedPropertySetter setter = 0, NamedPropertyQuery query = 0, NamedPropertyDeleter deleter = 0, NamedPropertyEnumerator enumerator = 0, Handle<Value> data = Handle<Value>()) {
    UNIMPLEMENTEDAPI();
  }

  void SetIndexedPropertyHandler(IndexedPropertyGetter getter, IndexedPropertySetter setter = 0, IndexedPropertyQuery query = 0, IndexedPropertyDeleter deleter = 0, IndexedPropertyEnumerator enumerator = 0, Handle<Value> data = Handle<Value>()) {
    UNIMPLEMENTEDAPI();
  }

  void SetCallAsFunctionHandler(InvocationCallback callback, Handle<Value> data = Handle<Value>()) {
    UNIMPLEMENTEDAPI();
  }

  void MarkAsUndetectable() {
    UNIMPLEMENTEDAPI();
  }

  void SetAccessCheckACalback(NamedSecurityCallback named_handler, IndexedSecurityCallback indexed_callback, Handle<Value> data = Handle<Value>(), bool turned_on_by_default = true) {
    UNIMPLEMENTEDAPI();
  }

  int InternalFieldCount() {
    UNIMPLEMENTEDAPI(0);
  }

  void SetInternalFieldCount(int i) {
    UNIMPLEMENTEDAPI();
  }
};

}
