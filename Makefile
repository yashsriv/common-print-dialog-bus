CC = g++
CFLAGS = -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -ldbus-1
DEPS = introspect.hpp printdialog.hpp manager.hpp
OBJ = introspect.o printdialog.o manager.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main.out: main.cpp $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
