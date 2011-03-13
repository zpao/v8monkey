#include "v8api_test_harness.h"

void
test_obj_setprop()
{
  HandleScope handle_scope;

  Persistent<Context> context = Context::New();

  Context::Scope context_scope(context);

  Handle<Object> obj = Object::New();

  Handle<String> k = String::New("someprop");
  Handle<String> v = String::New("somevalue");

  obj->Set(k, v);

  Handle<Value> v2 = obj->Get(k);

  String::AsciiValue vs(v);
  String::AsciiValue vs2(v2);

  do_check_eq(vs.length(), vs2.length());
  do_check_eq(0, strcmp(*vs,*vs2));

  context.Dispose();
}

void
test_obj_getprop()
{
  HandleScope handle_scope;

  Persistent<Context> context = Context::New();

  Context::Scope context_scope(context);

  Handle<Object> obj = Object::New();

  Handle<Value> k = String::New("toString");
  Handle<Value> v = obj->Get(k);

  do_check_true(v->IsFunction());

  context.Dispose();
}

////////////////////////////////////////////////////////////////////////////////
//// Test Harness

Test gTests[] = {
  TEST(test_obj_setprop),
  TEST(test_obj_getprop),
};

const char* file = __FILE__;
#define TEST_NAME "Object Properties"
#define TEST_FILE file
#include "v8api_test_harness_tail.h"
