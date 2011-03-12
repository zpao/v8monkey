#include "v8-internal.h"

namespace v8 {
  namespace internal {
    const int KB = 1024;
    const int MB = 1024 * 1024;
    JSRuntime *gRuntime = 0;
    JSRuntime *rt() {
      if (!gRuntime) {
        JS_CStringsAreUTF8();
        gRuntime = JS_NewRuntime(64 * MB);
      }
      return gRuntime;
    }

    // TODO: call this
    void shutdown() {
      if (gRuntime)
        JS_DestroyRuntime(gRuntime);
      JS_ShutDown();
    }

    JSClass global_class = {
      "global", JSCLASS_GLOBAL_FLAGS,
      JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
      JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
      JSCLASS_NO_OPTIONAL_MEMBERS
    };

    void reportError(JSContext *ctx, const char *message, JSErrorReport *report) {
      fprintf(stderr, "%s:%u:%s\n",
              report->filename ? report->filename : "<no filename>",
              (unsigned int) report->lineno,
              message);
    }
  }
}
