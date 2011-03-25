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

typedef JSUint16 uint16_t;
typedef JSInt32 int32_t;
typedef JSUint32 uint32_t;
typedef JSInt64 int64_t;

////////////////////////////////////////////////////////////////////////////////
//// Helpers

static inline v8::Local<v8::Number> v8_num(double x) {
  return v8::Number::New(x);
}
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

static int StrCmp16(uint16_t* a, uint16_t* b) {
  while (true) {
    if (*a == 0 && *b == 0) return 0;
    if (*a != *b) return 0 + *a - *b;
    a++;
    b++;
  }
}

static int StrNCmp16(uint16_t* a, uint16_t* b, int n) {
  while (true) {
    if (n-- == 0) return 0;
    if (*a == 0 && *b == 0) return 0;
    if (*a != *b) return 0 + *a - *b;
    a++;
    b++;
  }
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

void
test_GetSetProperty() {
  v8::HandleScope scope;
  LocalContext context;
  context->Global()->Set(v8_str("foo"), v8_num(14));
  context->Global()->Set(v8_str("12"), v8_num(92));
  context->Global()->Set(v8::Integer::New(16), v8_num(32));
  context->Global()->Set(v8_num(13), v8_num(56));
  Local<Value> foo = Script::Compile(v8_str("this.foo"))->Run();
  CHECK_EQ(14, foo->Int32Value());
  Local<Value> twelve = Script::Compile(v8_str("this[12]"))->Run();
  CHECK_EQ(92, twelve->Int32Value());
  Local<Value> sixteen = Script::Compile(v8_str("this[16]"))->Run();
  CHECK_EQ(32, sixteen->Int32Value());
  Local<Value> thirteen = Script::Compile(v8_str("this[13]"))->Run();
  CHECK_EQ(56, thirteen->Int32Value());
  CHECK_EQ(92, context->Global()->Get(v8::Integer::New(12))->Int32Value());
  CHECK_EQ(92, context->Global()->Get(v8_str("12"))->Int32Value());
  CHECK_EQ(92, context->Global()->Get(v8_num(12))->Int32Value());
  CHECK_EQ(32, context->Global()->Get(v8::Integer::New(16))->Int32Value());
  CHECK_EQ(32, context->Global()->Get(v8_str("16"))->Int32Value());
  CHECK_EQ(32, context->Global()->Get(v8_num(16))->Int32Value());
  CHECK_EQ(56, context->Global()->Get(v8::Integer::New(13))->Int32Value());
  CHECK_EQ(56, context->Global()->Get(v8_str("13"))->Int32Value());
  CHECK_EQ(56, context->Global()->Get(v8_num(13))->Int32Value());
}

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

void
test_TinyInteger()
{
  v8::HandleScope scope;
  LocalContext env;
  int32_t value = 239;
  Local<v8::Integer> value_obj = v8::Integer::New(value);
  CHECK_EQ(static_cast<int64_t>(value), value_obj->Value());
}

void
test_TinyUnsignedInteger()
{
  v8::HandleScope scope;
  LocalContext env;
  uint32_t value = 239;
  Local<v8::Integer> value_obj = v8::Integer::NewFromUnsigned(value);
  CHECK_EQ(static_cast<int64_t>(value), value_obj->Value());
}

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

void
test_Number()
{
  v8::HandleScope scope;
  LocalContext env;
  double PI = 3.1415926;
  Local<v8::Number> pi_obj = v8::Number::New(PI);
  CHECK_EQ(PI, pi_obj->NumberValue());
}

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

void
test_ConversionNumber()
{
  v8::HandleScope scope;
  LocalContext env;
  // Very large number.
  CompileRun("var obj = Math.pow(2,32) * 1237;");
  Local<Value> obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(5312874545152.0, obj->ToNumber()->Value());
  CHECK_EQ(0, obj->ToInt32()->Value());
  CHECK(0u == obj->ToUint32()->Value());  // NOLINT - no CHECK_EQ for unsigned.
  // Large number.
  CompileRun("var obj = -1234567890123;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(-1234567890123.0, obj->ToNumber()->Value());
  CHECK_EQ(-1912276171, obj->ToInt32()->Value());
  CHECK(2382691125u == obj->ToUint32()->Value());  // NOLINT
  // Small positive integer.
  CompileRun("var obj = 42;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(42.0, obj->ToNumber()->Value());
  CHECK_EQ(42, obj->ToInt32()->Value());
  CHECK(42u == obj->ToUint32()->Value());  // NOLINT
  // Negative integer.
  CompileRun("var obj = -37;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(-37.0, obj->ToNumber()->Value());
  CHECK_EQ(-37, obj->ToInt32()->Value());
  CHECK(4294967259u == obj->ToUint32()->Value());  // NOLINT
  // Positive non-int32 integer.
  CompileRun("var obj = 0x81234567;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(2166572391.0, obj->ToNumber()->Value());
  CHECK_EQ(-2128394905, obj->ToInt32()->Value());
  CHECK(2166572391u == obj->ToUint32()->Value());  // NOLINT
  // Fraction.
  CompileRun("var obj = 42.3;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(42.3, obj->ToNumber()->Value());
  CHECK_EQ(42, obj->ToInt32()->Value());
  CHECK(42u == obj->ToUint32()->Value());  // NOLINT
  // Large negative fraction.
  CompileRun("var obj = -5726623061.75;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK_EQ(-5726623061.75, obj->ToNumber()->Value());
  CHECK_EQ(-1431655765, obj->ToInt32()->Value());
  CHECK(2863311531u == obj->ToUint32()->Value());  // NOLINT
}

void
test_isNumberType() {
  v8::HandleScope scope;
  LocalContext env;
  // Very large number.
  CompileRun("var obj = Math.pow(2,32) * 1237;");
  Local<Value> obj = env->Global()->Get(v8_str("obj"));
  CHECK(!obj->IsInt32());
  CHECK(!obj->IsUint32());
  // Large negative number.
  CompileRun("var obj = -1234567890123;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK(!obj->IsInt32());
  CHECK(!obj->IsUint32());
  // Small positive integer.
  CompileRun("var obj = 42;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK(obj->IsInt32());
  CHECK(obj->IsUint32());
  // Negative integer.
  CompileRun("var obj = -37;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK(obj->IsInt32());
  CHECK(!obj->IsUint32());
  // Positive non-int32 integer.
  CompileRun("var obj = 0x81234567;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK(!obj->IsInt32());
  CHECK(obj->IsUint32());
  // Fraction.
  CompileRun("var obj = 42.3;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK(!obj->IsInt32());
  CHECK(!obj->IsUint32());
  // Large negative fraction.
  CompileRun("var obj = -5726623061.75;");
  obj = env->Global()->Get(v8_str("obj"));
  CHECK(!obj->IsInt32());
  CHECK(!obj->IsUint32());
}

void
test_StringWrite() {
  v8::HandleScope scope;
  v8::Handle<String> str = v8_str("abcde");
  // abc<Icelandic eth><Unicode snowman>.
  v8::Handle<String> str2 = v8_str("abc\303\260\342\230\203");

  CHECK_EQ(5, str2->Length());

  char buf[100];
  char utf8buf[100];
  uint16_t wbuf[100];
  int len;
  int charlen;

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, sizeof(utf8buf), &charlen);
  CHECK_EQ(len, 9);
  CHECK_EQ(charlen, 5);
  CHECK_EQ(strcmp(utf8buf, "abc\303\260\342\230\203"), 0);

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 8, &charlen);
  CHECK_EQ(len, 8);
  CHECK_EQ(charlen, 5);
  CHECK_EQ(strncmp(utf8buf, "abc\303\260\342\230\203\1", 9), 0);

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 7, &charlen);
  CHECK_EQ(len, 5);
  CHECK_EQ(charlen, 4);
  CHECK_EQ(strncmp(utf8buf, "abc\303\260\1", 5), 0);

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 6, &charlen);
  CHECK_EQ(len, 5);
  CHECK_EQ(charlen, 4);
  CHECK_EQ(strncmp(utf8buf, "abc\303\260\1", 5), 0);

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 5, &charlen);
  CHECK_EQ(len, 5);
  CHECK_EQ(charlen, 4);
  CHECK_EQ(strncmp(utf8buf, "abc\303\260\1", 5), 0);

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 4, &charlen);
  CHECK_EQ(len, 3);
  CHECK_EQ(charlen, 3);
  CHECK_EQ(strncmp(utf8buf, "abc\1", 4), 0);

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 3, &charlen);
  CHECK_EQ(len, 3);
  CHECK_EQ(charlen, 3);
  CHECK_EQ(strncmp(utf8buf, "abc\1", 4), 0);

  memset(utf8buf, 0x1, sizeof(utf8buf));
  len = str2->WriteUtf8(utf8buf, 2, &charlen);
  CHECK_EQ(len, 2);
  CHECK_EQ(charlen, 2);
  CHECK_EQ(strncmp(utf8buf, "ab\1", 3), 0);

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf);
  CHECK_EQ(len, 5);
  len = str->Write(wbuf);
  CHECK_EQ(len, 5);
  CHECK_EQ(strcmp("abcde", buf), 0);
  uint16_t answer1[] = {'a', 'b', 'c', 'd', 'e', '\0'};
  CHECK_EQ(StrCmp16(answer1, wbuf), 0);

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 0, 4);
  CHECK_EQ(len, 4);
  len = str->Write(wbuf, 0, 4);
  CHECK_EQ(len, 4);
  CHECK_EQ(strncmp("abcd\1", buf, 5), 0);
  uint16_t answer2[] = {'a', 'b', 'c', 'd', 0x101};
  CHECK_EQ(StrNCmp16(answer2, wbuf, 5), 0);

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 0, 5);
  CHECK_EQ(len, 5);
  len = str->Write(wbuf, 0, 5);
  CHECK_EQ(len, 5);
  CHECK_EQ(strncmp("abcde\1", buf, 6), 0);
  uint16_t answer3[] = {'a', 'b', 'c', 'd', 'e', 0x101};
  CHECK_EQ(StrNCmp16(answer3, wbuf, 6), 0);

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 0, 6);
  CHECK_EQ(len, 5);
  len = str->Write(wbuf, 0, 6);
  CHECK_EQ(len, 5);
  CHECK_EQ(strcmp("abcde", buf), 0);
  uint16_t answer4[] = {'a', 'b', 'c', 'd', 'e', '\0'};
  CHECK_EQ(StrCmp16(answer4, wbuf), 0);

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 4, -1);
  CHECK_EQ(len, 1);
  len = str->Write(wbuf, 4, -1);
  CHECK_EQ(len, 1);
  CHECK_EQ(strcmp("e", buf), 0);
  uint16_t answer5[] = {'e', '\0'};
  CHECK_EQ(StrCmp16(answer5, wbuf), 0);

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 4, 6);
  CHECK_EQ(len, 1);
  len = str->Write(wbuf, 4, 6);
  CHECK_EQ(len, 1);
  CHECK_EQ(strcmp("e", buf), 0);
  CHECK_EQ(StrCmp16(answer5, wbuf), 0);

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 4, 1);
  CHECK_EQ(len, 1);
  len = str->Write(wbuf, 4, 1);
  CHECK_EQ(len, 1);
  CHECK_EQ(strncmp("e\1", buf, 2), 0);
  uint16_t answer6[] = {'e', 0x101};
  CHECK_EQ(StrNCmp16(answer6, wbuf, 2), 0);

  memset(buf, 0x1, sizeof(buf));
  memset(wbuf, 0x1, sizeof(wbuf));
  len = str->WriteAscii(buf, 3, 1);
  CHECK_EQ(len, 1);
  len = str->Write(wbuf, 3, 1);
  CHECK_EQ(len, 1);
  CHECK_EQ(strncmp("d\1", buf, 2), 0);
  uint16_t answer7[] = {'d', 0x101};
  CHECK_EQ(StrNCmp16(answer7, wbuf, 2), 0);
}
////////////////////////////////////////////////////////////////////////////////
//// Test Harness

Test gTests[] = {
  DISABLED_TEST(test_Handles, 11),
  TEST(test_Access),
  TEST(test_GetSetProperty),
  TEST(test_Script),
  TEST(test_TinyInteger),
  TEST(test_TinyUnsignedInteger),
  TEST(test_OutOfSignedRangeUnsignedInteger),
  TEST(test_Number),
  TEST(test_ToNumber),
  DISABLED_TEST(test_Boolean, 10),
  DISABLED_TEST(test_HulIgennem, 12),
  TEST(test_AccessElement),
  TEST(test_ConversionNumber),
  TEST(test_isNumberType),
  DISABLED_TEST(test_StringWrite, 16),
};

const char* file = __FILE__;
#define TEST_NAME "V8 API"
#define TEST_FILE file
#include "v8api_test_harness_tail.h"
