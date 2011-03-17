#include "v8-internal.h"
#include <algorithm>

namespace v8 {
using namespace internal;

String::String(JSString *s) {
  mVal = STRING_TO_JSVAL(s);
}

Local<String> String::New(const char *data,
                          int length)
{
  if (length == -1) {
    length = strlen(data);
  }

  JSString *str = JS_NewStringCopyN(cx()->getJSContext(), data, length);
  String s(str);
  return Local<String>::New(&s);
}

int String::Length() const {
  return JS_GetStringLength(*this);
}

int String::Utf8Length() const {
  size_t encodedLength = JS_GetStringEncodingLength(cx()->getJSContext(),
                                                    *this);
  return static_cast<int>(encodedLength);
}

int String::Write(JSUint16* buffer,
                  int start,
                  int length,
                  WriteHints hints) const
{
  size_t internalLen;
  const jschar* chars =
    JS_GetStringCharsZAndLength(cx()->getJSContext(), *this, &internalLen);
  if (!chars || internalLen >= static_cast<size_t>(start)) {
    return 0;
  }
  size_t bytes = std::min<size_t>(length, internalLen - start) * 2;
  if (length == -1) {
    bytes = (internalLen - start) * 2;
  }
  (void)memcpy(buffer, &chars[start], bytes);

  return bytes / 2;
}

int String::WriteAscii(char* buffer,
                       int start,
                       int length,
                       WriteHints hints) const
{
  size_t encodedLength = JS_GetStringEncodingLength(cx()->getJSContext(),
                                                    *this);
  char* tmp = new char[encodedLength];
  int written = WriteUtf8(tmp, encodedLength);

  // No easy way to convert UTF-8 to ASCII, so just drop characters that are
  // not ASCII.
  int toWrite = start + length - 1;
  if (length == -1) {
    length = written + 1;
    toWrite = start + written;
  }
  int idx = 0;
  for (int i = start; i < toWrite; i++) {
    if (static_cast<unsigned int>(tmp[i]) > 0x7F) {
      continue;
    }
    buffer[idx] = tmp[i];
    idx++;
  }
  // If we have enough space for the NULL terminator, set it.
  if (idx <= length) {
    buffer[idx--] = '\0';
  }

  delete[] tmp;
  return idx - 1;
}

int String::WriteUtf8(char* buffer,
                      int length,
                      int* nchars_ref,
                      WriteHints hints) const
{
  // TODO handle -1 for length!
  // XXX unclear if this includes the NULL terminator!
  size_t bytesWritten = JS_EncodeStringToBuffer(*this, buffer, length);

  // If we have enough space for the NULL terminator, set it.
  if (bytesWritten < size_t(length)) {
    buffer[bytesWritten + 1] = '\0';
  }

  // TODO check to make sure we don't overflow int
  if (nchars_ref) {
    *nchars_ref = static_cast<int>(bytesWritten);
  }
  return static_cast<int>(bytesWritten);
}

String::AsciiValue::AsciiValue(Handle<v8::Value> val) {
  // Set some defaults which will be used for empty values/strings
  mStr = NULL;
  mLength = 0;

  if (val.IsEmpty()) {
    return;
  }

  // TODO: Do we need something about HandleScope here?
  Local<String> str = val->ToString();
  if (!str.IsEmpty()) {
    int len = str->Length();
    // Need space for the NULL terminator.
    mStr = new char[len + 1];
    mLength = str->WriteAscii(mStr, 0, len + 1);
  }
}
String::AsciiValue::~AsciiValue() {
  if (mStr)
    delete[] mStr;
}
}
