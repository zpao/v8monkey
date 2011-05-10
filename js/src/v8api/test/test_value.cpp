/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */

#include "v8api_test_harness.h"

static inline Local<Value> CompileRun(const char* source) {
  return Script::Compile(String::New(source))->Run();
}

////////////////////////////////////////////////////////////////////////////////
//// Tests

void
test_ToInteger()
{
  HandleScope handle_scope;
  Persistent<Context> context = Context::New();
  Context::Scope context_scope(context);

  // Convert string to Integer.
  Handle<Value> result = CompileRun("'123'");
  Local<Integer> integer = result->ToInteger();
  do_check_eq(integer->Value(), 123);

  // Convert integer number to Integer.
  result = CompileRun("123");
  integer = result->ToInteger();
  do_check_eq(integer->Value(), 123);

  // Convert decimal number to Integer.
  result = CompileRun("123.45");
  integer = result->ToInteger();
  do_check_eq(integer->Value(), 123);

  // Convert negative number to Integer.
  result = CompileRun("-123");
  integer = result->ToInteger();
  do_check_eq(integer->Value(), -123);

/* TODO: we don't support 64 bit numbers yet

  // Convert huge number to Integer.
  result = CompileRun("0x7fffffffffffffff");
  integer = result->ToInteger();
  do_check_eq(integer->Value(), (JSInt64)0x7fffffffffffffff);
*/
}


////////////////////////////////////////////////////////////////////////////////
//// Test Harness

Test gTests[] = {
  TEST(test_ToInteger),
};

const char* file = __FILE__;
#define TEST_NAME "Value Class"
#define TEST_FILE file
#include "v8api_test_harness_tail.h"
