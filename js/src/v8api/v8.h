#include "jsapi.h"
#include "jstl.h"
#include "jshashtable.h"

namespace v8 {
// Define some classes first so we can use them before fully defined
struct HandleScope;
class Boolean;
class Number;
class Integer;
class String;
class Array;
class Object;
class Uint32;
class Int32;
class Context;
class Function;
class ScriptOrigin;
class AccessorInfo;
class FunctionTemplate;
class ObjectTemplate;
class Signature;
template <class T> class Handle;
template <class T> class Local;
template <class T> class Persistent;

#define UNIMPLEMENTEDAPI(...) \
  JS_BEGIN_MACRO \
  v8::internal::notImplemented(); \
  return __VA_ARGS__; \
  JS_END_MACRO

namespace internal {
class GCReference;
struct GCOps;
class GCReferenceContainer;
struct RCOps;
class RCReferenceContainer;

void notImplemented();

class GCReference {
  friend struct GCOps;

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

// Hash policy for jsids
struct JSIDHashPolicy
{
  typedef jsid Key;
  typedef Key Lookup;

  static uint32 hash(const Lookup &l) {
    return *reinterpret_cast<const uint32*>(&l);
  }

  static JSBool match(const Key &k, const Lookup &l) {
    return k == l;
  }
};
} // namespace internal

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
  T* operator ->() const {
    return mVal;
  }
  T* operator *() const {
    return mVal;
  }

  template <class S>
  inline Handle<S> As() {
    return Handle<S>::Cast(*this);
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

  template <class S>
  static inline Local<T> Cast(Local<S> that) {
    return Local<T>(T::Cast(*that));
  }

  template <class S>
  inline Local<S> As() {
    return Local<S>::Cast(*this);
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

class Context : public internal::GCReference {
  Context(JSObject *global);
public:
  void Enter();
  void Exit();

  inline Local<Object> Global();

  static Local<Context> GetEntered();
  static Local<Context> GetCurrent();

  static Persistent<Context> New();

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

  operator JSObject*() const { return JSVAL_TO_OBJECT(mVal); }
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
  Local<Object> ToObject() const;
  Local<Uint32> ToUint32() const;
  Local<Int32> ToInt32() const;

  bool BooleanValue() const;
  double NumberValue() const;
  JSInt64 IntegerValue() const;
  JSUint32 Uint32Value() const;
  JSInt32 Int32Value() const;

  bool Equals(Handle<Value> other) const;
  bool StrictEquals(Handle<Value> other) const;
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
protected:
  Number(jsval v) {
    mVal = v;
  }
public:
  double Value() const;
  static Local<Number> New(double value);
  static Number* Cast(v8::Value* obj) {
    UNIMPLEMENTEDAPI(NULL);
  }
};

class Integer : public Number {
protected:
  Integer(jsval v) : Number(v) { }
public:
  static Local<Integer> New(JSInt32 value);
  static Local<Integer> NewFromUnsigned(JSUint32 value);
  JSInt64 Value() const;
  // XXX Cast is inline in V8
  static Integer* Cast(v8::Value* obj) {
    // TODO: check for uint32
    if (obj->IsInt32()) {
      return reinterpret_cast<Integer*>(obj);
    }
    return NULL;
  }
};

class Int32 : public Integer {
  friend class Value;
  Int32(JSInt32 i) : Integer(INT_TO_JSVAL(i)) { }
public:
  JSInt32 Value();
};

class Uint32 : public Integer {
  friend class Value;
  Uint32(JSUint32 i) : Integer(UINT_TO_JSVAL(i)) { }
public:
  JSUint32 Value();
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
  static Local<String> FromJSID(jsid id);
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

enum ExternalArrayType {
  kExternalByteArray = 1,
  kExternalUnsignedByteArray,
  kExternalShortArray,
  kExternalUnsignedShortArray,
  kExternalIntArray,
  kExternalUnsignedIntArray,
  kExternalFloatArray
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
  struct PrivateData;
  PrivateData& GetHiddenStore();
  friend class Context;
  friend class Script;
  friend class ObjectTemplate;
  friend class Arguments;
protected:
  operator JSObject *() const { return JSVAL_TO_OBJECT(mVal); }
  Object(JSObject *obj);
public:
  bool Set(Handle<Value> key, Handle<Value> value, PropertyAttribute attribs = None);
  bool Set(JSUint32 index, Handle<Value> value);
  bool ForceSet(Handle<Value> key, Handle<Value> value, PropertyAttribute attrib = None);
  Local<Value> Get(Handle<Value> key);
  Local<Value> Get(JSUint32 index);
  bool Has(Handle<String> key);
  bool Has(JSUint32 index);
  bool Delete(Handle<String> key);
  bool Delete(JSUint32 index);
  bool ForceDelete(Handle<String> key);
  bool SetAccessor(Handle<String> name, AccessorGetter getter, AccessorSetter setter = 0, Handle<Data> data = Handle<Data>(), AccessControl settings = DEFAULT, PropertyAttribute attribs = None);
  Local<Array> GetPropertyNames();
  Local<Value> GetPrototype();
  void SetPrototype(Handle<Value> prototype);
  Local<Object> FindInstanceInPrototypeChain(Handle<FunctionTemplate> tmpl);
  Local<String> ObjectProtoToString();
  Local<String> GetConstructorName();
  int InternalFieldCount();
  Local<Value> GetInternalField(int index);
  void SetInternalField(int index, Handle<Value> value);
  void* GetPointerFromInternalField(int index);
  void SetPointerInInternalField(int index, void* value);
  bool HasRealNamedProperty(Handle<String> key);
  bool HasRealIndexedProperty(JSUint32 index);
  bool HasRealNamedCallbackProperty(Handle<String> key);
  Local<Value> GetRealNamedPropertyInPrototypeChain(Handle<String> key);
  Local<Value> GetRealNamedProperty(Handle<String> key);
  bool HasNamedLookupInterceptor();
  bool HasIndexedLookupInterceptor();
  void TurnOnAccessCheck();
  int GetIdentityHash();
  bool SetHiddenValue(Handle<String> key, Handle<Value> value);
  Local<Value> GetHiddenValue(Handle<String> key);
  bool DeleteHiddenValue(Handle<String> key);
  bool IsDirty();
  Local<Object> Clone();
  void SetIndexedPropertiesToPixelData(JSUint8* data, int length);
  bool HasIndexedPropertiesInPixelData();
  JSUint8* GetIndexedPropertiesPixelData();
  int GetIndexedPropertiesPixelDataLength();
  void SetIndexedPropertiesToExternalArrayData(
      void* data,
      ExternalArrayType array_type,
      int number_of_elements);
  bool HasIndexedPropertiesInExternalArrayData();
  void* GetIndexedPropertiesExternalArrayData();
  ExternalArrayType GetIndexedPropertiesExternalArrayDataType();
  int GetIndexedPropertiesExternalArrayDataLength();

  static Local<Object> New();
};

class Array : public Object {
  Array(JSObject* obj) :
    Object(obj)
  {}
 public:
  JSUint32 Length() const;
  Local<Object> CloneElementAt(JSUint32 index);

  static Local<Array> New(int length = 0);
  static inline Array* Cast(Value* obj) {
    if (obj->IsArray()) {
      return reinterpret_cast<Array*>(obj);
    }
    return NULL;
  }
};

class Function : public Object {
  Function(JSObject *obj) :
    Object(obj)
  {}
  Function(JSFunction *fn) :
    Object(JS_GetFunctionObject(fn))
  {}

  friend class Object;
  friend class Arguments;
public:
  Local<Object> NewInstance() const {
    UNIMPLEMENTEDAPI(NULL);
  }
  Local<Object> NewInstance(int argc, Handle<Value> argv[]) const {
    UNIMPLEMENTEDAPI(NULL);
  }
  Local<Value> Call(Handle<Object> recv, int argc, Handle<Value> argv[]) const {
    UNIMPLEMENTEDAPI(NULL);
  }
  void SetName(Handle<String> name);
  Handle<String> GetName() const;

  int GetScriptLineNumber() const {
    UNIMPLEMENTEDAPI(0);
  }
  // UNIMPLEMENTEDAPI
  ScriptOrigin GetScriptOrigin() const;
  static const int kLineOffsetNotFound;
};

class ScriptOrigin {
  Handle<Value> mResourceName;
  Handle<Integer> mResourceLineOffset;
  Handle<Integer> mResourceColumnOffset;

public:
  // XXX: V8 inlines these, should we?
  ScriptOrigin(Handle<Value> resourceName,
               Handle<Integer> resourceLineOffset = Handle<Integer>(),
               Handle<Integer> resourceColumnOffset = Handle<Integer>());
  Handle<Value> ResourceName() const;
  Handle<Integer> ResourceLineOffset() const;
  Handle<Integer> ResourceColumnOffset() const;
};

class ScriptData {
public:
  virtual ~ScriptData() { }
  static ScriptData* PreCompile(const char* input, int length);
  static ScriptData* PreCompile(Handle<String> source);
  static ScriptData* New(const char* data, int length);
  virtual int Length() = 0;
  virtual const char* Data() = 0;
  virtual bool HasError() = 0;
};

class Script : public internal::GCReference {
  Script(JSScript *s);

  operator JSScript *();
public:
  static Local<Script> New(Handle<String> source, ScriptOrigin *origin = NULL,
                           ScriptData *preData = NULL,
                           Handle<String> scriptData = Handle<String>());
  static Local<Script> New(Handle<String> source, Handle<Value> fileName);
  static Local<Script> Compile(Handle<String> source, ScriptOrigin *origin = NULL,
                               ScriptData *preData = NULL,
                               Handle<String> scriptData = Handle<String>());
  static Local<Script> Compile(Handle<String> source,
                               Handle<Value> fileName,
                               Handle<String> scriptData = Handle<String>());
  Local<Value> Run();
  Local<Value> Id();
};

class Template : public Data {
public:
  void Set(Handle<String> name, Handle<Data> value,
           PropertyAttribute attribs = None);

  inline void Set(const char* name, Handle<Data> value);
private:
  Template();

  struct PrivateData;
  typedef js::HashMap<jsid, PrivateData, internal::JSIDHashPolicy, js::SystemAllocPolicy> TemplateHash;
  TemplateHash mData;

  friend class FunctionTemplate;
  friend class ObjectTemplate;
};

class Arguments {
  friend class Object;
  Arguments(JSContext* cx, JSObject* thisObj, int nargs, jsval* vals, Handle<Value> data);

  JSContext *mCtx;
  jsval *mValues;
  JSObject *mThis;
  int mLength;
  Local<Value> mData;
public:
  int Length() const {
    return mLength;
  }
  Local<Value> operator[](int i) const;
  Local<Function> Callee() const;

  Local<Object> This() const {
    Object o(mThis);
    return Local<Object>::New(&o);
  }
  Local<Object> Holder() const {
    UNIMPLEMENTEDAPI(NULL);
  }
  bool IsConstructCall() const;
  Local<Value> Data() const {
    return mData;
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

typedef Handle<Value> (*IndexedPropertyGetter)(JSUint32 index, const AccessorInfo &info);
typedef Handle<Value> (*IndexedPropertySetter)(JSUint32 index, Local<Value> value, const AccessorInfo &info);
typedef Handle<Integer> (*IndexedPropertyQuery)(JSUint32 index, const AccessorInfo &info);
typedef Handle<Boolean> (*IndexedPropertyDeleter)(JSUint32 index, AccessorInfo &info);
typedef Handle<Array> (*IndexedPropertyEnumerator)(const AccessorInfo &info);

enum AccessType {
  ACCESS_GET,
  ACCESS_SET,
  ACCESS_HAS,
  ACCESS_DELETE,
  ACCESS_KEYS
};

typedef bool (*NamedSecurityCallback)(Local<Object> host, Local<Value> key, AccessType type, Local<Value> data);
typedef bool (*IndexedSecurityCallback)(Local<Object> host, JSUint32 index, AccessType type, Local<Value> data);

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
  ObjectTemplate();
public:
  static Local<ObjectTemplate> New();
  Local<Object> NewInstance();
  void SetAccessor(Handle<String> name, AccessorGetter getter, AccessorSetter setter, Handle<Value> data = Handle<Value>(), AccessControl settings = DEFAULT, PropertyAttribute attribs = None);
  void SetNamedPropertyHandler(NamedPropertyGetter getter, NamedPropertySetter setter = 0, NamedPropertyQuery query = 0, NamedPropertyDeleter deleter = 0, NamedPropertyEnumerator enumerator = 0, Handle<Value> data = Handle<Value>());
  void SetIndexedPropertyHandler(IndexedPropertyGetter getter, IndexedPropertySetter setter = 0, IndexedPropertyQuery query = 0, IndexedPropertyDeleter deleter = 0, IndexedPropertyEnumerator enumerator = 0, Handle<Value> data = Handle<Value>());
  void SetCallAsFunctionHandler(InvocationCallback callback, Handle<Value> data = Handle<Value>());
  void MarkAsUndetectable();
  void SetAccessCheckCallbacks(NamedSecurityCallback named_handler, IndexedSecurityCallback indexed_callback, Handle<Value> data = Handle<Value>(), bool turned_on_by_default = true);
  int InternalFieldCount();
  void SetInternalFieldCount(int value);
};

Local<Object> Context::Global() {
  Object obj(*this);
  return Local<Object>::New(&obj);
}

} // namespace v8

namespace i = v8::internal;
