// Copyright 2007-2009 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * This test file is an adaptation of test-api.cc from the v8 source tree.  We
 * would need to import a whole bunch of their source files to just drop in that
 * test file, which we don't really want to do.
 */

#include "v8api_test_harness.h"

#define CHECK do_check_true
#define CHECK_EQ(expected, actual) do_check_eq(actual, expected)

typedef JSInt32 int32_t;
typedef JSUint32 uint32_t;
typedef JSInt64 int64_t;

////////////////////////////////////////////////////////////////////////////////
//// Helpers

static inline v8::Local<v8::String> v8_str(const char* x) {
  return v8::String::New(x);
}

static inline v8::Local<v8::Script> v8_compile(const char* x) {
  return v8::Script::Compile(v8_str(x));
}

// Helper function that compiles and runs the source.
static inline v8::Local<v8::Value> CompileRun(const char* source) {
  return v8::Script::Compile(v8::String::New(source))->Run();
}

static void ExpectString(const char* code, const char* expected) {
  Local<Value> result = CompileRun(code);
  CHECK(result->IsString());
  String::AsciiValue ascii(result);
  CHECK_EQ(expected, *ascii);
}

static void ExpectBoolean(const char* code, bool expected) {
  Local<Value> result = CompileRun(code);
  CHECK(result->IsBoolean());
  CHECK_EQ(expected, result->BooleanValue());
}

static void ExpectTrue(const char* code) {
  ExpectBoolean(code, true);
}

static void ExpectFalse(const char* code) {
  ExpectBoolean(code, false);
}

static void ExpectObject(const char* code, Local<Value> expected) {
  Local<Value> result = CompileRun(code);
  CHECK(result->Equals(expected));
}

static void ExpectUndefined(const char* code) {
  Local<Value> result = CompileRun(code);
  CHECK(result->IsUndefined());
}

// A LocalContext holds a reference to a v8::Context.
class LocalContext {
 public:
  // TODO use v8::ExtensionConfiguration again
  LocalContext(//v8::ExtensionConfiguration* extensions = 0,
               v8::Handle<v8::ObjectTemplate> global_template =
                   v8::Handle<v8::ObjectTemplate>(),
               v8::Handle<v8::Value> global_object = v8::Handle<v8::Value>())
    : context_(v8::Context::New()) {
//    : context_(v8::Context::New(extensions, global_template, global_object)) {
    context_->Enter();
  }

  virtual ~LocalContext() {
    context_->Exit();
    context_.Dispose();
  }

  v8::Context* operator->() { return *context_; }
  v8::Context* operator*() { return *context_; }
  bool IsReady() { return !context_.IsEmpty(); }

  v8::Local<v8::Context> local() {
    return v8::Local<v8::Context>::New(context_);
  }

 private:
  v8::Persistent<v8::Context> context_;
};

namespace v8 {
namespace internal {
template <typename T>
static T* NewArray(int size) {
  T* result = new T[size];
  do_check_true(result);
  return result;
}


template <typename T>
static void DeleteArray(T* array) {
  delete[] array;
}
} // namespace internal
} // namespace v8

////////////////////////////////////////////////////////////////////////////////
//// Tests

// from test-api.cc:125
void
test_Handles()
{
  v8::HandleScope scope;
  Local<Context> local_env;
  {
    LocalContext env;
    local_env = env.local();
  }

  // Local context should still be live.
  CHECK(!local_env.IsEmpty());
  local_env->Enter();

  v8::Handle<v8::Primitive> undef = v8::Undefined();
  CHECK(!undef.IsEmpty());
  CHECK(undef->IsUndefined());

  const char* c_source = "1 + 2 + 3";
  Local<String> source = String::New(c_source);
  Local<Script> script = Script::Compile(source);
  CHECK_EQ(6, script->Run()->Int32Value());

  local_env->Exit();
}

// from test-api.cc:150
void
test_ReceiverSignature()
{ }

// from test-api.cc:196
void
test_ArgumentSignature()
{ }

// from test-api.cc:258
void
test_HulIgennem()
{
  v8::HandleScope scope;
  LocalContext env;
  v8::Handle<v8::Primitive> undef = v8::Undefined();
  Local<String> undef_str = undef->ToString();
  char* value = i::NewArray<char>(undef_str->Length() + 1);
  undef_str->WriteAscii(value);
  CHECK_EQ(0, strcmp(value, "undefined"));
  i::DeleteArray(value);
}

// from test-api.cc:270
void
test_Access()
{
  v8::HandleScope scope;
  LocalContext env;
  Local<v8::Object> obj = v8::Object::New();
  Local<Value> foo_before = obj->Get(v8_str("foo"));
  CHECK(foo_before->IsUndefined());
  Local<String> bar_str = v8_str("bar");
  obj->Set(v8_str("foo"), bar_str);
  Local<Value> foo_after = obj->Get(v8_str("foo"));
  CHECK(!foo_after->IsUndefined());
  CHECK(foo_after->IsString());
  CHECK_EQ(bar_str, foo_after);
}

// from test-api.cc:285
void
test_AccessElement()
{
  v8::HandleScope scope;
  LocalContext env;
  Local<v8::Object> obj = v8::Object::New();
  Local<Value> before = obj->Get(1);
  CHECK(before->IsUndefined());
  Local<String> bar_str = v8_str("bar");
  obj->Set(1, bar_str);
  Local<Value> after = obj->Get(1);
  CHECK(!after->IsUndefined());
  CHECK(after->IsString());
  CHECK_EQ(bar_str, after);

  Local<v8::Array> value = CompileRun("[\"a\", \"b\"]").As<v8::Array>();
  CHECK_EQ(v8_str("a"), value->Get(0));
  CHECK_EQ(v8_str("b"), value->Get(1));
}

// from test-api.cc:304
void
test_Script()
{
  v8::HandleScope scope;
  LocalContext env;
  const char* c_source = "1 + 2 + 3";
  Local<String> source = String::New(c_source);
  Local<Script> script = Script::Compile(source);
  CHECK_EQ(6, script->Run()->Int32Value());
}

// from test-api.cc:381
void
test_ScriptUsingStringResource()
{ }

// from test-api.cc:406
void
test_ScriptUsingAsciiStringResource()
{ }

// from test-api.cc:427
void
test_ScriptMakingExternalString()
{ }

// from test-api.cc:452
void
test_ScriptMakingExternalAsciiString()
{ }

// from test-api.cc:478
void
test_MakingExternalStringConditions()
{ }

// from test-api.cc:524
void
test_MakingExternalAsciiStringConditions()
{ }

// from test-api.cc:561
void
test_UsingExternalString()
{ }

// from test-api.cc:579
void
test_UsingExternalAsciiString()
{ }

// from test-api.cc:597
void
test_ScavengeExternalString()
{ }

// from test-api.cc:616
void
test_ScavengeExternalAsciiString()
{ }

// from test-api.cc:655
void
test_ExternalStringWithDisposeHandling()
{ }

// from test-api.cc:701
void
test_StringConcat()
{ }

// from test-api.cc:747
void
test_GlobalProperties()
{ }

// from test-api.cc:776
void
test_FunctionTemplate()
{ }

// from test-api.cc:844
void
test_ExternalWrap()
{ }

// from test-api.cc:890
void
test_FindInstanceInPrototypeChain()
{ }

// from test-api.cc:937
void
test_TinyInteger()
{
  v8::HandleScope scope;
  LocalContext env;
  int32_t value = 239;
  Local<v8::Integer> value_obj = v8::Integer::New(value);
  CHECK_EQ(static_cast<int64_t>(value), value_obj->Value());
}

// from test-api.cc:946
void
test_BigSmiInteger()
{ }

// from test-api.cc:960
void
test_BigInteger()
{ }

// from test-api.cc:977
void
test_TinyUnsignedInteger()
{
  v8::HandleScope scope;
  LocalContext env;
  uint32_t value = 239;
  Local<v8::Integer> value_obj = v8::Integer::NewFromUnsigned(value);
  CHECK_EQ(static_cast<int64_t>(value), value_obj->Value());
}

// from test-api.cc:986
void
test_BigUnsignedSmiInteger()
{ }

// from test-api.cc:997
void
test_BigUnsignedInteger()
{ }

// from test-api.cc:1008
void
test_OutOfSignedRangeUnsignedInteger()
{
  v8::HandleScope scope;
  LocalContext env;
  uint32_t INT32_MAX_AS_UINT = (1U << 31) - 1;
  uint32_t value = INT32_MAX_AS_UINT + 1;
  CHECK(value > INT32_MAX_AS_UINT);  // No overflow.
  Local<v8::Integer> value_obj = v8::Integer::NewFromUnsigned(value);
  CHECK_EQ(static_cast<int64_t>(value), value_obj->Value());
}

// from test-api.cc:1019
void
test_Number()
{
  v8::HandleScope scope;
  LocalContext env;
  double PI = 3.1415926;
  Local<v8::Number> pi_obj = v8::Number::New(PI);
  CHECK_EQ(PI, pi_obj->NumberValue());
}

// from test-api.cc:1028
void
test_ToNumber()
{
  v8::HandleScope scope;
  LocalContext env;
  Local<String> str = v8_str("3.1415926");
  CHECK_EQ(3.1415926, str->NumberValue());
  v8::Handle<v8::Boolean> t = v8::True();
  CHECK_EQ(1.0, t->NumberValue());
  v8::Handle<v8::Boolean> f = v8::False();
  CHECK_EQ(0.0, f->NumberValue());
}

// from test-api.cc:1040
void
test_Date()
{ }

// from test-api.cc:1049
void
test_Boolean()
{
  v8::HandleScope scope;
  LocalContext env;
  v8::Handle<v8::Boolean> t = v8::True();
  CHECK(t->Value());
  v8::Handle<v8::Boolean> f = v8::False();
  CHECK(!f->Value());
  v8::Handle<v8::Primitive> u = v8::Undefined();
  CHECK(!u->BooleanValue());
  v8::Handle<v8::Primitive> n = v8::Null();
  CHECK(!n->BooleanValue());
  v8::Handle<String> str1 = v8_str("");
  CHECK(!str1->BooleanValue());
  v8::Handle<String> str2 = v8_str("x");
  CHECK(str2->BooleanValue());
  CHECK(!v8::Number::New(0)->BooleanValue());
  CHECK(v8::Number::New(-1)->BooleanValue());
  CHECK(v8::Number::New(1)->BooleanValue());
  CHECK(v8::Number::New(42)->BooleanValue());
  CHECK(!v8_compile("NaN")->Run()->BooleanValue());
}

// from test-api.cc:1084
void
test_GlobalPrototype()
{ }

// from test-api.cc:1103
void
test_ObjectTemplate()
{ }

// from test-api.cc:1139
void
test_DescriptorInheritance()
{ }

// from test-api.cc:1207
void
test_NamedPropertyHandlerGetter()
{ }

// from test-api.cc:1243
void
test_IndexedPropertyHandlerGetter()
{ }

// from test-api.cc:1344
void
test_PropertyHandlerInPrototype()
{ }

// from test-api.cc:1413
void
test_PrePropertyHandler()
{ }

// from test-api.cc:1431
void
test_UndefinedIsNotEnumerable()
{ }

// from test-api.cc:1468
void
test_DeepCrossLanguageRecursion()
{ }

// from test-api.cc:1502
void
test_CallbackExceptionRegression()
{ }

// from test-api.cc:1518
void
test_FunctionPrototype()
{ }

// from test-api.cc:1529
void
test_InternalFields()
{ }

// from test-api.cc:1544
void
test_GlobalObjectInternalFields()
{ }

// from test-api.cc:1558
void
test_InternalFieldsNativePointers()
{ }

// from test-api.cc:1590
void
test_InternalFieldsNativePointersAndExternal()
{ }

// from test-api.cc:1628
void
test_IdentityHash()
{ }

// from test-api.cc:1672
void
test_HiddenProperties()
{ }

// from test-api.cc:1728
void
test_HiddenPropertiesWithInterceptors()
{ }

// from test-api.cc:1748
void
test_External()
{ }

// from test-api.cc:1780
void
test_GlobalHandle()
{ }

// from test-api.cc:1792
void
test_ScriptException()
{ }

// from test-api.cc:1817
void
test_MessageHandlerData()
{ }

// from test-api.cc:1835
void
test_GetSetProperty()
{ }

// from test-api.cc:1862
void
test_PropertyAttributes()
{ }

// from test-api.cc:1882
void
test_Array()
{ }

// from test-api.cc:1916
void
test_Vector()
{ }

// from test-api.cc:1954
void
test_FunctionCall()
{ }

// from test-api.cc:2012
void
test_OutOfMemory()
{ }

// from test-api.cc:2053
void
test_OutOfMemoryNested()
{ }

// from test-api.cc:2082
void
test_HugeConsStringOutOfMemory()
{ }

// from test-api.cc:2108
void
test_ConstructCall()
{ }

// from test-api.cc:2168
void
test_ConversionNumber()
{ }

// from test-api.cc:2216
void
test_isNumberType()
{ }

// from test-api.cc:2257
void
test_ConversionException()
{ }

// from test-api.cc:2327
void
test_APICatch()
{ }

// from test-api.cc:2345
void
test_APIThrowTryCatch()
{ }

// from test-api.cc:2364
void
test_TryCatchInTryFinally()
{ }

// from test-api.cc:2399
void
test_APIThrowMessageOverwrittenToString()
{ }

// from test-api.cc:2444
void
test_APIThrowMessage()
{ }

// from test-api.cc:2458
void
test_APIThrowMessageAndVerboseTryCatch()
{ }

// from test-api.cc:2476
void
test_ExternalScriptException()
{ }

// from test-api.cc:2543
void
test_EvalInTryFinally()
{ }

// from test-api.cc:2578
void
test_ExceptionOrder()
{ }

// from test-api.cc:2642
void
test_ThrowValues()
{ }

// from test-api.cc:2668
void
test_CatchZero()
{ }

// from test-api.cc:2684
void
test_CatchExceptionFromWith()
{ }

// from test-api.cc:2694
void
test_TryCatchAndFinallyHidingException()
{ }

// from test-api.cc:2711
void
test_TryCatchAndFinally()
{ }

// from test-api.cc:2729
void
test_Equality()
{ }

// from test-api.cc:2761
void
test_MultiRun()
{ }

// from test-api.cc:2779
void
test_SimplePropertyRead()
{ }

// from test-api.cc:2792
void
test_DefinePropertyOnAPIAccessor()
{ }

// from test-api.cc:2840
void
test_DefinePropertyOnDefineGetterSetter()
{ }

// from test-api.cc:2893
void
test_DefineAPIAccessorOnObject()
{ }

// from test-api.cc:2967
void
test_DontDeleteAPIAccessorsCannotBeOverriden()
{ }

// from test-api.cc:3025
void
test_ElementAPIAccessor()
{ }

// from test-api.cc:3063
void
test_SimplePropertyWrite()
{ }

// from test-api.cc:3088
void
test_NamedInterceptorPropertyRead()
{ }

// from test-api.cc:3102
void
test_NamedInterceptorDictionaryIC()
{ }

// from test-api.cc:3132
void
test_NamedInterceptorDictionaryICMultipleContext()
{ }

// from test-api.cc:3186
void
test_NamedInterceptorMapTransitionRead()
{ }

// from test-api.cc:3223
void
test_IndexedInterceptorWithIndexedAccessor()
{ }

// from test-api.cc:3260
void
test_IndexedInterceptorWithGetOwnPropertyDescriptor()
{ }

// from test-api.cc:3281
void
test_IndexedInterceptorWithNoSetter()
{ }

// from test-api.cc:3304
void
test_IndexedInterceptorWithAccessorCheck()
{ }

// from test-api.cc:3328
void
test_IndexedInterceptorWithAccessorCheckSwitchedOn()
{ }

// from test-api.cc:3358
void
test_IndexedInterceptorWithDifferentIndices()
{ }

// from test-api.cc:3381
void
test_IndexedInterceptorWithNegativeIndices()
{ }

// from test-api.cc:3420
void
test_IndexedInterceptorWithNotSmiLookup()
{ }

// from test-api.cc:3449
void
test_IndexedInterceptorGoingMegamorphic()
{ }

// from test-api.cc:3479
void
test_IndexedInterceptorReceiverTurningSmi()
{ }

// from test-api.cc:3509
void
test_IndexedInterceptorOnProto()
{ }

// from test-api.cc:3533
void
test_MultiContexts()
{ }

// from test-api.cc:3566
void
test_FunctionPrototypeAcrossContexts()
{ }

// from test-api.cc:3596
void
test_Regress892105()
{ }

// from test-api.cc:3619
void
test_UndetectableObject()
{ }

// from test-api.cc:3664
void
test_ExtensibleOnUndetectable()
{ }

// from test-api.cc:3699
void
test_UndetectableString()
{ }

// from test-api.cc:3764
void
test_GlobalObjectTemplate()
{ }

// from test-api.cc:3782
void
test_SimpleExtensions()
{ }

// from test-api.cc:3811
void
test_UseEvalFromExtension()
{ }

// from test-api.cc:3844
void
test_UseWithFromExtension()
{ }

// from test-api.cc:3859
void
test_AutoExtensions()
{ }

// from test-api.cc:3877
void
test_SyntaxErrorExtensions()
{ }

// from test-api.cc:3894
void
test_ExceptionExtensions()
{ }

// from test-api.cc:3915
void
test_NativeCallInExtensions()
{ }

// from test-api.cc:3943
void
test_ExtensionDependency()
{ }

// from test-api.cc:4010
void
test_FunctionLookup()
{ }

// from test-api.cc:4023
void
test_NativeFunctionConstructCall()
{ }

// from test-api.cc:4055
void
test_ErrorReporting()
{ }

// from test-api.cc:4082
void
test_RegexpOutOfMemory()
{ }

// from test-api.cc:4106
void
test_ErrorWithMissingScriptInfo()
{ }

// from test-api.cc:4169
void
test_WeakReference()
{ }

// from test-api.cc:4222
void
test_NoWeakRefCallbacksInScavenge()
{ }

// from test-api.cc:4268
void
test_Arguments()
{ }

// from test-api.cc:4309
void
test_Deleter()
{ }

// from test-api.cc:4372
void
test_Enumerators()
{ }

// from test-api.cc:4486
void
test_GetterHolders()
{ }

// from test-api.cc:4499
void
test_PreInterceptorHolders()
{ }

// from test-api.cc:4509
void
test_ObjectInstantiation()
{ }

// from test-api.cc:4549
void
test_StringWrite()
{ }

// from test-api.cc:4692
void
test_ToArrayIndex()
{ }

// from test-api.cc:4723
void
test_ErrorConstruction()
{ }

// from test-api.cc:4764
void
test_DeleteAccessor()
{ }

// from test-api.cc:4777
void
test_TypeSwitch()
{ }

// from test-api.cc:4854
void
test_ApiUncaughtException()
{ }

// from test-api.cc:4894
void
test_ExceptionInNativeScript()
{ }

// from test-api.cc:4914
void
test_CompilationErrorUsingTryCatchHandler()
{ }

// from test-api.cc:4924
void
test_TryCatchFinallyUsingTryCatchHandler()
{ }

// from test-api.cc:4945
void
test_SecurityHandler()
{ }

// from test-api.cc:5007
void
test_SecurityChecks()
{ }

// from test-api.cc:5052
void
test_SecurityChecksForPrototypeChain()
{ }

// from test-api.cc:5120
void
test_CrossDomainDelete()
{ }

// from test-api.cc:5153
void
test_CrossDomainIsPropertyEnumerable()
{ }

// from test-api.cc:5188
void
test_CrossDomainForIn()
{ }

// from test-api.cc:5221
void
test_ContextDetachGlobal()
{ }

// from test-api.cc:5285
void
test_DetachAndReattachGlobal()
{ }

// from test-api.cc:5411
void
test_AccessControl()
{ }

// from test-api.cc:5655
void
test_AccessControlES5()
{ }

// from test-api.cc:5721
void
test_AccessControlGetOwnPropertyNames()
{ }

// from test-api.cc:5771
void
test_GetOwnPropertyNamesWithInterceptor()
{ }

// from test-api.cc:5795
void
test_CrossDomainAccessors()
{ }

// from test-api.cc:5870
void
test_AccessControlIC()
{ }

// from test-api.cc:6019
void
test_AccessControlFlatten()
{ }

// from test-api.cc:6083
void
test_AccessControlInterceptorIC()
{ }

// from test-api.cc:6150
void
test_Version()
{ }

// from test-api.cc:6161
void
test_InstanceProperties()
{ }

// from test-api.cc:6190
void
test_GlobalObjectInstanceProperties()
{ }

// from test-api.cc:6246
void
test_CallKnownGlobalReceiver()
{ }

// from test-api.cc:6323
void
test_ShadowObject()
{ }

// from test-api.cc:6368
void
test_HiddenPrototype()
{ }

// from test-api.cc:6412
void
test_SetPrototype()
{ }

// from test-api.cc:6469
void
test_SetPrototypeThrows()
{ }

// from test-api.cc:6490
void
test_GetterSetterExceptions()
{ }

// from test-api.cc:6513
void
test_Constructor()
{ }

// from test-api.cc:6526
void
test_FunctionDescriptorException()
{ }

// from test-api.cc:6551
void
test_EvalAliasedDynamic()
{ }

// from test-api.cc:6585
void
test_CrossEval()
{ }

// from test-api.cc:6668
void
test_EvalInDetachedGlobal()
{ }

// from test-api.cc:6703
void
test_CrossLazyLoad()
{ }

// from test-api.cc:6738
void
test_CallAsFunction()
{ }

// from test-api.cc:6804
void
test_HandleIteration()
{ }

// from test-api.cc:6839
void
test_InterceptorHasOwnProperty()
{ }

// from test-api.cc:6871
void
test_InterceptorHasOwnPropertyCausingGC()
{ }

// from test-api.cc:6927
void
test_InterceptorLoadIC()
{ }

// from test-api.cc:6949
void
test_InterceptorLoadICWithFieldOnHolder()
{ }

// from test-api.cc:6960
void
test_InterceptorLoadICWithSubstitutedProto()
{ }

// from test-api.cc:6971
void
test_InterceptorLoadICWithPropertyOnProto()
{ }

// from test-api.cc:6982
void
test_InterceptorLoadICUndefined()
{ }

// from test-api.cc:6992
void
test_InterceptorLoadICWithOverride()
{ }

// from test-api.cc:7012
void
test_InterceptorLoadICFieldNotNeeded()
{ }

// from test-api.cc:7032
void
test_InterceptorLoadICInvalidatedField()
{ }

// from test-api.cc:7063
void
test_InterceptorLoadICPostInterceptor()
{ }

// from test-api.cc:7088
void
test_InterceptorLoadICInvalidatedFieldViaGlobal()
{ }

// from test-api.cc:7113
void
test_InterceptorLoadICWithCallbackOnHolder()
{ }

// from test-api.cc:7142
void
test_InterceptorLoadICWithCallbackOnProto()
{ }

// from test-api.cc:7175
void
test_InterceptorLoadICForCallbackWithOverride()
{ }

// from test-api.cc:7203
void
test_InterceptorLoadICCallbackNotNeeded()
{ }

// from test-api.cc:7231
void
test_InterceptorLoadICInvalidatedCallback()
{ }

// from test-api.cc:7263
void
test_InterceptorLoadICInvalidatedCallbackViaGlobal()
{ }

// from test-api.cc:7299
void
test_InterceptorReturningZero()
{ }

// from test-api.cc:7315
void
test_InterceptorStoreIC()
{ }

// from test-api.cc:7330
void
test_InterceptorStoreICWithNoSetter()
{ }

// from test-api.cc:7360
void
test_InterceptorCallIC()
{ }

// from test-api.cc:7379
void
test_InterceptorCallICSeesOthers()
{ }

// from test-api.cc:7407
void
test_InterceptorCallICCacheableNotNeeded()
{ }

// from test-api.cc:7427
void
test_InterceptorCallICInvalidatedCacheable()
{ }

// from test-api.cc:7465
void
test_InterceptorCallICConstantFunctionUsed()
{ }

// from test-api.cc:7486
void
test_InterceptorCallICConstantFunctionNotNeeded()
{ }

// from test-api.cc:7508
void
test_InterceptorCallICInvalidatedConstantFunction()
{ }

// from test-api.cc:7538
void
test_InterceptorCallICInvalidatedConstantFunctionViaGlobal()
{ }

// from test-api.cc:7563
void
test_InterceptorCallICCachedFromGlobal()
{ }

// from test-api.cc:7641
void
test_CallICFastApi_DirectCall_GCMoveStub()
{ }

// from test-api.cc:7665
void
test_CallICFastApi_DirectCall_Throw()
{ }

// from test-api.cc:7696
void
test_LoadICFastApi_DirectCall_GCMoveStub()
{ }

// from test-api.cc:7718
void
test_LoadICFastApi_DirectCall_Throw()
{ }

// from test-api.cc:7734
void
test_InterceptorCallICFastApi_TrivialSignature()
{ }

// from test-api.cc:7761
void
test_InterceptorCallICFastApi_SimpleSignature()
{ }

// from test-api.cc:7791
void
test_InterceptorCallICFastApi_SimpleSignature_Miss1()
{ }

// from test-api.cc:7827
void
test_InterceptorCallICFastApi_SimpleSignature_Miss2()
{ }

// from test-api.cc:7863
void
test_InterceptorCallICFastApi_SimpleSignature_Miss3()
{ }

// from test-api.cc:7902
void
test_InterceptorCallICFastApi_SimpleSignature_TypeError()
{ }

// from test-api.cc:7941
void
test_CallICFastApi_TrivialSignature()
{ }

// from test-api.cc:7964
void
test_CallICFastApi_SimpleSignature()
{ }

// from test-api.cc:7990
void
test_CallICFastApi_SimpleSignature_Miss1()
{ }

// from test-api.cc:8021
void
test_CallICFastApi_SimpleSignature_Miss2()
{ }

// from test-api.cc:8070
void
test_InterceptorKeyedCallICKeyChange1()
{ }

// from test-api.cc:8094
void
test_InterceptorKeyedCallICKeyChange2()
{ }

// from test-api.cc:8121
void
test_InterceptorKeyedCallICKeyChangeOnGlobal()
{ }

// from test-api.cc:8146
void
test_InterceptorKeyedCallICFromGlobal()
{ }

// from test-api.cc:8170
void
test_InterceptorKeyedCallICMapChangeBefore()
{ }

// from test-api.cc:8192
void
test_InterceptorKeyedCallICMapChangeAfter()
{ }

// from test-api.cc:8228
void
test_InterceptorICReferenceErrors()
{ }

// from test-api.cc:8274
void
test_InterceptorICGetterExceptions()
{ }

// from test-api.cc:8317
void
test_InterceptorICSetterExceptions()
{ }

// from test-api.cc:8336
void
test_NullNamedInterceptor()
{ }

// from test-api.cc:8351
void
test_NullIndexedInterceptor()
{ }

// from test-api.cc:8365
void
test_NamedPropertyHandlerGetterAttributes()
{ }

// from test-api.cc:8391
void
test_Overriding()
{ }

// from test-api.cc:8457
void
test_IsConstructCall()
{ }

// from test-api.cc:8474
void
test_ObjectProtoToString()
{ }

// from test-api.cc:8508
void
test_ObjectGetConstructorName()
{ }

// from test-api.cc:8659
void
test_Threading()
{ }

// from test-api.cc:8665
void
test_Threading2()
{ }

// from test-api.cc:8721
void
test_NestedLockers()
{ }

// from test-api.cc:8743
void
test_NestedLockersNoTryCatch()
{ }

// from test-api.cc:8763
void
test_RecursiveLocking()
{ }

// from test-api.cc:8779
void
test_LockUnlockLock()
{ }

// from test-api.cc:8836
void
test_DontLeakGlobalObjects()
{ }

// from test-api.cc:8880
void
test_NewPersistentHandleFromWeakCallback()
{ }

// from test-api.cc:8909
void
test_DoNotUseDeletedNodesInSecondLevelGc()
{ }

// from test-api.cc:8934
void
test_NoGlobalHandlesOrphaningDueToWeakCallback()
{ }

// from test-api.cc:8950
void
test_CheckForCrossContextObjectLiterals()
{ }

// from test-api.cc:8983
void
test_NestedHandleScopeAndContexts()
{ }

// from test-api.cc:8994
void
test_ExternalAllocatedMemory()
{ }

// from test-api.cc:9003
void
test_DisposeEnteredContext()
{ }

// from test-api.cc:9018
void
test_Regress54()
{ }

// from test-api.cc:9035
void
test_CatchStackOverflow()
{ }

// from test-api.cc:9072
void
test_TryCatchSourceInfo()
{ }

// from test-api.cc:9108
void
test_CompilationCache()
{ }

// from test-api.cc:9131
void
test_CallbackFunctionName()
{ }

// from test-api.cc:9144
void
test_DateAccess()
{ }

// from test-api.cc:9164
void
test_PropertyEnumeration()
{ }

// from test-api.cc:9209
void
test_DisableAccessChecksWhileConfiguring()
{ }

// from test-api.cc:9240
void
test_AccessChecksReenabledCorrectly()
{ }

// from test-api.cc:9279
void
test_AccessControlRepeatedContextCreation()
{ }

// from test-api.cc:9295
void
test_TurnOnAccessCheck()
{ }

// from test-api.cc:9371
void
test_TurnOnAccessCheckAndRecompile()
{ }

// from test-api.cc:9462
void
test_PreCompile()
{ }

// from test-api.cc:9476
void
test_PreCompileWithError()
{ }

// from test-api.cc:9486
void
test_Regress31661()
{ }

// from test-api.cc:9497
void
test_PreCompileSerialization()
{ }

// from test-api.cc:9523
void
test_PreCompileDeserializationError()
{ }

// from test-api.cc:9536
void
test_PreCompileInvalidPreparseDataError()
{ }

// from test-api.cc:9584
void
test_PreCompileAPIVariationsAreSame()
{ }

// from test-api.cc:9622
void
test_DictionaryICLoadedFunction()
{ }

// from test-api.cc:9643
void
test_CrossContextNew()
{ }

// from test-api.cc:9775
void
test_RegExpInterruption()
{ }

// from test-api.cc:9883
void
test_ApplyInterruption()
{ }

// from test-api.cc:9905
void
test_ObjectClone()
{ }

// from test-api.cc:9988
void
test_MorphCompositeStringTest()
{ }

// from test-api.cc:10041
void
test_CompileExternalTwoByteSource()
{ }

// from test-api.cc:10181
void
test_RegExpStringModification()
{ }

// from test-api.cc:10204
void
test_ReadOnlyPropertyInGlobalProto()
{ }

// from test-api.cc:10252
void
test_ForceSet()
{ }

// from test-api.cc:10293
void
test_ForceSetWithInterceptor()
{ }

// from test-api.cc:10340
void
test_ForceDelete()
{ }

// from test-api.cc:10375
void
test_ForceDeleteWithInterceptor()
{ }

// from test-api.cc:10410
void
test_ForceDeleteIC()
{ }

// from test-api.cc:10447
void
test_GetCallingContext()
{ }

// from test-api.cc:10500
void
test_InitGlobalVarInProtoChain()
{ }

// from test-api.cc:10516
void
test_ReplaceConstantFunction()
{ }

// from test-api.cc:10530
void
test_Regress16276()
{ }

// from test-api.cc:10547
void
test_PixelArray()
{ }

// from test-api.cc:10921
void
test_PixelArrayInfo()
{ }

// from test-api.cc:10953
void
test_PixelArrayWithInterceptor()
{ }

// from test-api.cc:11361
void
test_ExternalByteArray()
{ }

// from test-api.cc:11369
void
test_ExternalUnsignedByteArray()
{ }

// from test-api.cc:11377
void
test_ExternalShortArray()
{ }

// from test-api.cc:11385
void
test_ExternalUnsignedShortArray()
{ }

// from test-api.cc:11393
void
test_ExternalIntArray()
{ }

// from test-api.cc:11401
void
test_ExternalUnsignedIntArray()
{ }

// from test-api.cc:11409
void
test_ExternalFloatArray()
{ }

// from test-api.cc:11417
void
test_ExternalArrays()
{ }

// from test-api.cc:11446
void
test_ExternalArrayInfo()
{ }

// from test-api.cc:11457
void
test_ScriptContextDependence()
{ }

// from test-api.cc:11473
void
test_StackTrace()
{ }

// from test-api.cc:11562
void
test_CaptureStackTrace()
{ }

// from test-api.cc:11620
void
test_CaptureStackTraceForUncaughtException()
{ }

// from test-api.cc:11643
void
test_CaptureStackTraceForUncaughtExceptionAndSetters()
{ }

// from test-api.cc:11679
void
test_SourceURLInStackTrace()
{ }

// from test-api.cc:11703
void
test_IdleNotification()
{ }

// from test-api.cc:11736
void
test_SetResourceConstraints()
{ }

// from test-api.cc:11758
void
test_SetResourceConstraintsInThread()
{ }

// from test-api.cc:11788
void
test_GetHeapStatistics()
{ }

// from test-api.cc:11843
void
test_QuietSignalingNaNs()
{ }

// from test-api.cc:11927
void
test_SpaghettiStackReThrow()
{ }

// from test-api.cc:11953
void
test_Regress528()
{ }

// from test-api.cc:12042
void
test_ScriptOrigin()
{ }

// from test-api.cc:12064
void
test_ScriptLineNumber()
{ }

// from test-api.cc:12093
void
test_SetterOnConstructorPrototype()
{ }

// from test-api.cc:12143
void
test_InterceptorOnConstructorPrototype()
{ }

// from test-api.cc:12177
void
test_Bug618()
{ }

// from test-api.cc:12243
void
test_GCCallbacks()
{ }

// from test-api.cc:12277
void
test_AddToJSFunctionResultCache()
{ }

// from test-api.cc:12304
void
test_FillJSFunctionResultCache()
{ }

// from test-api.cc:12326
void
test_RoundRobinGetFromCache()
{ }

// from test-api.cc:12351
void
test_ReverseGetFromCache()
{ }

// from test-api.cc:12376
void
test_TestEviction()
{ }

// from test-api.cc:12394
void
test_TwoByteStringInAsciiCons()
{ }

// from test-api.cc:12483
void
test_GCInFailedAccessCheckCallback()
{ }

// from test-api.cc:12561
void
test_StringCheckMultipleContexts()
{ }

// from test-api.cc:12584
void
test_NumberCheckMultipleContexts()
{ }

// from test-api.cc:12607
void
test_BooleanCheckMultipleContexts()
{ }

// from test-api.cc:12630
void
test_DontDeleteCellLoadIC()
{ }

// from test-api.cc:12670
void
test_DontDeleteCellLoadICForceDelete()
{ }

// from test-api.cc:12698
void
test_DontDeleteCellLoadICAPI()
{ }

// from test-api.cc:12726
void
test_GlobalLoadICGC()
{ }

// from test-api.cc:12768
void
test_RegExp()
{ }

// from test-api.cc:12833
void
test_Equals()
{ }

// from test-api.cc:12865
void
test_NamedEnumeratorAndForIn()
{ }

// from test-api.cc:12880
void
test_DefinePropertyPostDetach()
{ }


////////////////////////////////////////////////////////////////////////////////
//// Test Harness

Test gTests[] = {
  DISABLED_TEST(test_Handles),
  TEST(test_Access),
  DISABLED_TEST(test_Script),
  TEST(test_TinyInteger),
  TEST(test_TinyUnsignedInteger),
  TEST(test_OutOfSignedRangeUnsignedInteger),
  DISABLED_TEST(test_Number),
  DISABLED_TEST(test_ToNumber),
  DISABLED_TEST(test_Boolean),
  DISABLED_TEST(test_HulIgennem),
  DISABLED_TEST(test_AccessElement),
};

const char* file = __FILE__;
#define TEST_NAME "V8 API"
#define TEST_FILE file
#include "v8api_test_harness_tail.h"
