#ifndef v8_v8_internal_h__
#define v8_v8_internal_h__
#include "v8.h"

namespace v8 {
namespace internal {

// Core
JSRuntime *rt();
void shutdown();
void reportError(JSContext *ctx, const char *message, JSErrorReport *report);
extern JSClass global_class;

JSContext *cx();

////////////////////////////////////////////////////////////////////////////////
//// Debug Helpers

#ifdef DEBUG
void Dump(Handle<Object> obj);
void Dump(Object& obj);
void Dump(Handle<String> str);
void Dump(String& str);
#endif // DEBUG

} // namespace internal
} // namespace v8

#endif // v8_v8_internal_h__
