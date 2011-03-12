#include "v8api_test_harness.h"

void
test_script_run_void()
{
  // Create a stack-allocated handle scope.
  HandleScope handle_scope;

  // Create a new context.
  Persistent<Context> context = Context::New();

  // Enter the created context for compiling.
  Context::Scope context_scope(context);

  // Create a string containing the JavaScript source code.
  Handle<String> source = String::New("oasdohuasdnlqwoi");

  // Compile the source code.
  Handle<Script> script = Script::Compile(source);

  // Run the script to get the result.
  Handle<Value> result = script->Run();

  // Dispose the persistent context.
  context.Dispose();

  do_check_eq(result->native(), JSVAL_VOID);
}

////////////////////////////////////////////////////////////////////////////////
//// Test Harness

Test gTests[] = {
  TEST(test_script_run_void),
};

const char* file = __FILE__;
#define TEST_NAME "Script Run Void"
#define TEST_FILE file
#include "v8api_test_harness_tail.h"
