CC=gcc
LINK=ld
AR=ar -r
RM=rm -f
CFLAGS=-Ijsapi -I..

v8.a: v8.o
	$(AR) $@ v8.o

v8.o: v8.cpp v8.h jslib
	$(CC) -o $@ $(CFLAGS) -c v8.cpp

jslib: jsapi/libjs_static.a

jsapi/js-config.h:
	if [ ! -e jsapi ]; then mkdir jsapi; fi
	(cd jsapi; ../../configure)

jsapi/libjs_static.a: jsapi/js-config.h
	$(MAKE) -C jsapi

clean:
	$(RM) v8.a v8.o

.PHONY: clean jslib
