/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */

#include "v8api_test_harness.h"

////////////////////////////////////////////////////////////////////////////////
//// Tests

void
test_ArrayConversion()
{
  HandleScope handle_scope;
  Persistent<Context> context = Context::New();
  Context::Scope context_scope(context);

  Local<String> s = String::New("hey");
  Local<String> uncaught_exception_symbol_l = Local<String>::New(s);
  Local<Value> argv[1] = { uncaught_exception_symbol_l  };
}

////////////////////////////////////////////////////////////////////////////////
//// Test Harness

Test gTests[] = {
  TEST(test_ArrayConversion),
};

const char* file = __FILE__;
#define TEST_NAME "Handle Classes"
#define TEST_FILE file
#include "v8api_test_harness_tail.h"

