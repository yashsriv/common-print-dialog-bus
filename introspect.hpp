#ifndef INTROSPECT_H
#define INTROSPECT_H

#include <dbus/dbus.h>

#include <string>

std::string read_xml(std::string);
void introspect(DBusMessage*, DBusConnection*, std::string);

#endif
