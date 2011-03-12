#pragma once
#include "v8.h"

namespace v8 {
  namespace internal {
    // Core
    JSRuntime *rt();
    void shutdown();
    void reportError(JSContext *ctx, const char *message, JSErrorReport *report);
    extern JSClass global_class;

    // Persistent context for various manipulations
    Persistent<Context> gcx();
    Context *cx();
  }
}
