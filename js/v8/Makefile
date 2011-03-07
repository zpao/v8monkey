CC=gcc
LINK=ld
AR=ar -r
RM=rm -f

v8.a: v8.o
	$(AR) $@ v8.o

v8.o: v8.cpp v8.h
	$(CC) -o $@ -c v8.cpp

clean:
	$(RM) v8.a v8.o

.PHONY: clean
