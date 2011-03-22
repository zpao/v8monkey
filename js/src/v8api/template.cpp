#include "v8-internal.h"

namespace v8 {
using namespace internal;

struct Template::PrivateData {
  Persistent<Data> value;
  PropertyAttribute attribs;
};

Template::Template() :
  Data()
{
  (void)mData.init();
}

void
Template::Set(Handle<String> name,
              Handle<Data> value,
              PropertyAttribute attribs)
{
  jsid id;
  if (!JS_ValueToId(cx(), name->native(), &id)) {
    // TODO handle failures!
    UNIMPLEMENTEDAPI();
  }
  TemplateHash::AddPtr ref = mData.lookupForAdd(id);
  PrivateData pd = { Persistent<Data>::New(value), attribs };
  (void)mData.add(ref, id, pd);
}

void
Template::Set(const char* name,
              Handle<Data> value)
{
  Set(String::New(name), value);
}

} // namespace v8
