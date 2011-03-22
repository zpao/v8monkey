#include "v8-internal.h"
#include "jstl.h"
#include "jshashtable.h"
#include <limits>

namespace v8 {
using namespace internal;

struct PropertyData {
  AccessorGetter getter;
  AccessorSetter setter;
  Handle<Data> data;
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

typedef js::HashMap<jsid, PropertyData, JSIDHashPolicy, js::SystemAllocPolicy> AccessorTable;

struct Object::PrivateData {
  Persistent<Object> hiddenValues;
  AccessorTable properties;

  PrivateData() {
    properties.init(10);
  }
};

bool
Object::Set(Handle<Value> key,
            Handle<Value> value,
            PropertyAttribute attribs) {
  // TODO: use String::Value
  String::AsciiValue k(key);
  jsval v = value->native();

  if (!JS_SetProperty(cx(), *this, *k, &v)) {
    return false;
  }

  uintN js_attribs = 0;
  if (attribs & ReadOnly) {
    js_attribs |= JSPROP_READONLY;
  }
  if (!(attribs & DontEnum)) {
    js_attribs |= JSPROP_ENUMERATE;
  }
  if (attribs & DontDelete) {
    js_attribs |= JSPROP_PERMANENT;
  }

  JSBool wasFound;
  return JS_SetPropertyAttributes(cx(), *this, *k, js_attribs, &wasFound);
}

bool
Object::Set(JSUint32 index,
            Handle<Value> value)
{
  if (index > JSUint32(std::numeric_limits<jsint>::max())) {
    return false;
  }

  jsval& val = value->native();
  return !!JS_SetElement(cx(), *this, index, &val);
}

bool
Object::ForceSet(Handle<Value> key,
                 Handle<Value> value,
                 PropertyAttribute attribute)
{
  UNIMPLEMENTEDAPI(false);
}

Local<Value>
Object::Get(Handle<Value> key) {
  // TODO: use String::Value
  String::AsciiValue k(key);
  Value v(JSVAL_VOID);

  if (JS_GetProperty(cx(), *this, *k, &v.native())) {
    return Local<Value>::New(&v);
  }
  return Local<Value>();
}

Local<Value>
Object::Get(JSUint32 index)
{
  if (index > JSUint32(std::numeric_limits<jsint>::max())) {
    return Local<Value>();
  }

  Value v;
  if (JS_GetElement(cx(), *this, index, &v.native())) {
    return Local<Value>::New(&v);
  }
  return Local<Value>();
}

bool
Object::Has(Handle<String> key)
{
  // TODO: use String::Value
  String::AsciiValue k(key);

  JSBool found;
  if (JS_HasProperty(cx(), *this, *k, &found)) {
    return !!found;
  }
  return false;
}

bool
Object::Has(JSUint32 index)
{
  if (index > JSUint32(std::numeric_limits<jsint>::max())) {
    return false;
  }

  jsval val;
  if (JS_LookupElement(cx(), *this, index, &val)) {
    return !JSVAL_IS_VOID(val);
  }
  return false;
}

bool
Object::Delete(Handle<String> key)
{
  // TODO: use String::Value
  String::AsciiValue k(key);

  jsval val;
  if (JS_DeleteProperty2(cx(), *this, *k, &val)) {
    return val == JSVAL_TRUE;
  }
  return false;
}

bool
Object::Delete(JSUint32 index)
{
  if (index > JSUint32(std::numeric_limits<jsint>::max())) {
    return false;
  }

  jsval val;
  if (JS_DeleteElement2(cx(), *this, index, &val)) {
    return val == JSVAL_TRUE;
  }
  return false;
}

bool
Object::ForceDelete(Handle<String> key)
{
  UNIMPLEMENTEDAPI(false);
}

bool
Object::SetAccessor(Handle<String> name,
                    AccessorGetter getter,
                    AccessorSetter setter,
                    Handle<Data> data,
                    AccessControl settings,
                    PropertyAttribute attribs)
{
  UNIMPLEMENTEDAPI(false);
}

Local<Array>
Object::GetPropertyNames()
{
  UNIMPLEMENTEDAPI(NULL);
}

Local<Value>
Object::GetPrototype()
{
  UNIMPLEMENTEDAPI(NULL);
}

void
Object::SetPrototype(Handle<Value> prototype)
{
  UNIMPLEMENTEDAPI();
}

Local<Object>
Object::FindInstanceInPrototypeChain(Handle<FunctionTemplate> tmpl)
{
  UNIMPLEMENTEDAPI(NULL);
}

Local<String>
Object::ObjectProtoToString()
{
  UNIMPLEMENTEDAPI(NULL);
}

Local<String>
Object::GetConstructorName()
{
  UNIMPLEMENTEDAPI(NULL);
}

int
Object::InternalFieldCount()
{
  UNIMPLEMENTEDAPI(0);
}

Local<Value>
Object::GetInternalField(int index)
{
  UNIMPLEMENTEDAPI(NULL);
}

void
Object::SetInternalField(int index,
                         Handle<Value> value)
{
  UNIMPLEMENTEDAPI();
}

void*
Object::GetPointerFromInternalField(int index)
{
  UNIMPLEMENTEDAPI(NULL);
}

void
Object::SetPointerInInternalField(int index,
                                  void* value)
{
  UNIMPLEMENTEDAPI();
}

bool
Object::HasRealNamedProperty(Handle<String> key)
{
  // TODO: use String::Value
  String::AsciiValue k(key);

  JSBool found;
  if (JS_AlreadyHasOwnProperty(cx(), *this, *k, &found)) {
    return !!found;
  }
  return false;
}

bool
Object::HasRealIndexedProperty(JSUint32 index)
{
  if (index > JSUint32(std::numeric_limits<jsint>::max())) {
    return false;
  }

  JSBool found;
  if (JS_AlreadyHasOwnElement(cx(), *this, index, &found)) {
    return !!found;
  }
  return false;
}

bool
Object::HasRealNamedCallbackProperty(Handle<String> key)
{
  UNIMPLEMENTEDAPI(false);
}

Local<Value>
Object::GetRealNamedPropertyInPrototypeChain(Handle<String> key)
{
  UNIMPLEMENTEDAPI(NULL);
}

Local<Value>
Object::GetRealNamedProperty(Handle<String> key)
{
  UNIMPLEMENTEDAPI(NULL);
}

bool
Object::HasNamedLookupInterceptor()
{
  UNIMPLEMENTEDAPI(false);
}

bool
Object::HasIndexedLookupInterceptor()
{
  UNIMPLEMENTEDAPI(false);
}

void
Object::TurnOnAccessCheck()
{
  UNIMPLEMENTEDAPI();
}

int
Object::GetIdentityHash()
{
  UNIMPLEMENTEDAPI(0);
}

Object::PrivateData&
Object::GetHiddenStore()
{
  void* obj = JS_GetPrivate(cx(), *this);
  if (!obj) {
    // XXX this will leak.  Fix!
    PrivateData* pd = new PrivateData();
    (void)JS_SetPrivate(cx(), *this, pd);
    obj = pd;
  }
  return *reinterpret_cast<PrivateData*>(obj);
}

bool
Object::SetHiddenValue(Handle<String> key,
                       Handle<Value> value)
{
  PrivateData& pd = GetHiddenStore();
  return pd.hiddenValues->Set(key, value);
}

Local<Value>
Object::GetHiddenValue(Handle<String> key)
{
  PrivateData& pd = GetHiddenStore();
  return pd.hiddenValues->Get(key);
}

bool
Object::DeleteHiddenValue(Handle<String> key)
{
  PrivateData& pd = GetHiddenStore();
  return pd.hiddenValues->Delete(key);
}

bool
Object::IsDirty()
{
  UNIMPLEMENTEDAPI(false);
}

Local<Object>
Object::Clone()
{
  UNIMPLEMENTEDAPI(NULL);
}

void
Object::SetIndexedPropertiesToPixelData(JSUint8* data,
                                        int length)
{
  UNIMPLEMENTEDAPI();
}

bool
Object::HasIndexedPropertiesInPixelData()
{
  UNIMPLEMENTEDAPI(false);
}

JSUint8*
Object::GetIndexedPropertiesPixelData()
{
  UNIMPLEMENTEDAPI(NULL);
}

int
Object::GetIndexedPropertiesPixelDataLength()
{
  UNIMPLEMENTEDAPI(0);
}

void
Object::SetIndexedPropertiesToExternalArrayData(void* data,
                                                ExternalArrayType array_type,
                                                int number_of_elements)
{
  UNIMPLEMENTEDAPI();
}

bool
Object::HasIndexedPropertiesInExternalArrayData()
{
  UNIMPLEMENTEDAPI(false);
}

void*
Object::GetIndexedPropertiesExternalArrayData()
{
  UNIMPLEMENTEDAPI(NULL);
}

ExternalArrayType
Object::GetIndexedPropertiesExternalArrayDataType()
{
  UNIMPLEMENTEDAPI(kExternalByteArray);
}

int
Object::GetIndexedPropertiesExternalArrayDataLength()
{
  UNIMPLEMENTEDAPI(0);
}

Object::Object(JSObject *obj) :
  Value(OBJECT_TO_JSVAL(obj))
{
}

Local<Object>
Object::New() {
  JSObject *obj = JS_NewObject(cx(), NULL, NULL, NULL);
  if (!obj) {
    return Local<Object>();
  }
  Object o(obj);
  return Local<Object>::New(&o);
}

} /* namespace v8 */
