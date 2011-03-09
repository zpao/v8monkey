# Don't forget to add new compiled executables to .gitignore

INCLUDES=-I../jsapi/dist/include -I..
LIBRARIES=../v8.a ../jsapi/libjs_static.a
RM=rm -f

hello_world:
	g++ $(INCLUDES) -o $@ hello_world.cpp $(LIBRARIES)

clean:
	$(RM) hello_world
