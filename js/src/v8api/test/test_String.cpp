/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */

#include "v8api_test_harness.h"

void
test_Length()
{
  HandleScope handle_scope;
  Persistent<Context> context = Context::New();
  Context::Scope context_scope(context);

  char TEST_STRING[] = "this is a test";
  int TEST_LENGTH = strlen(TEST_STRING);
  Handle<String> str = String::New(TEST_STRING);
  do_check_eq(str->Length(), TEST_LENGTH);

  // Now, make a string with an embedded NULL.
  TEST_STRING[8] = '\0';
  str = String::New(TEST_STRING, TEST_LENGTH);
  do_check_eq(str->Length(), TEST_LENGTH);

  // Finally, make sure that we end up calling strlen if no length is passed.
  str = String::New(TEST_STRING);
  do_check_eq(str->Length(), strlen(TEST_STRING));
}

void
test_Utf8Length()
{
  HandleScope handle_scope;
  Persistent<Context> context = Context::New();
  Context::Scope context_scope(context);

  char TEST_STRING[] = "this is a UTF-8 test!  This is pi: π";
  int TEST_LENGTH = strlen(TEST_STRING);
  Handle<String> str = String::New(TEST_STRING);
  do_check_eq(str->Utf8Length(), TEST_LENGTH);

  // Now, make a string with an embedded NULL.
  TEST_STRING[8] = '\0';
  str = String::New(TEST_STRING, TEST_LENGTH);
  do_check_eq(str->Utf8Length(), TEST_LENGTH);
}

void
test_Write()
{
  HandleScope handle_scope;
  Persistent<Context> context = Context::New();
  Context::Scope context_scope(context);

}

void
test_WriteAscii()
{
  HandleScope handle_scope;
  Persistent<Context> context = Context::New();
  Context::Scope context_scope(context);

}

void
test_WriteUtf8()
{
  HandleScope handle_scope;
  Persistent<Context> context = Context::New();
  Context::Scope context_scope(context);

}

void
test_AsciiValue_operators()
{
  HandleScope handle_scope;
  Persistent<Context> context = Context::New();
  Context::Scope context_scope(context);

}

void
test_AsciiValue_length()
{
  HandleScope handle_scope;
  Persistent<Context> context = Context::New();
  Context::Scope context_scope(context);

}

////////////////////////////////////////////////////////////////////////////////
//// Test Harness

Test gTests[] = {
  TEST(test_Length),
  TEST(test_Utf8Length),
  TEST(test_Write),
  TEST(test_WriteAscii),
  TEST(test_WriteUtf8),
  TEST(test_AsciiValue_operators),
  TEST(test_AsciiValue_length),
};

const char* file = __FILE__;
#define TEST_NAME "String Classes"
#define TEST_FILE file
#include "v8api_test_harness_tail.h"
