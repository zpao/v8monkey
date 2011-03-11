#include "v8api_test_harness.h"

// Code is the sample code from https://code.google.com/apis/v8/get_started.html

void
test_hello_world_sample()
{
  // Create a stack-allocated handle scope.
  HandleScope handle_scope;

  // Create a new context.
  Persistent<Context> context = Context::New();

  // Enter the created context for compiling and
  // running the hello world script.
  Context::Scope context_scope(context);

  // Create a string containing the JavaScript source code.
  Handle<String> source = String::New("'Hello' + ', World!'");

  // Compile the source code.
  Handle<Script> script = Script::Compile(source);

  // Run the script to get the result.
  Handle<Value> result = script->Run();

  // Dispose the persistent context.
  context.Dispose();

  // Convert the result to an ASCII string and print it.
  String::AsciiValue ascii(result);
  do_check_eq(strcmp("Hello, World!", *ascii), 0);
}

////////////////////////////////////////////////////////////////////////////////
//// Test Harness

Test gTests[] = {
  TEST(test_hello_world_sample),
};

const char* file = __FILE__;
#define TEST_NAME "Hello World"
#define TEST_FILE file
#include "v8api_test_harness_tail.h"
