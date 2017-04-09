#include <dbus/dbus.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "introspect.hpp"
#include "manager.hpp"
#include "printdialog.hpp"

void listen() {
  DBusMessage* msg;
  DBusConnection* conn;
  DBusError err;

  int ret;

  std::cerr << "Listening for method calls\n";

  // initialise the error
  dbus_error_init(&err);

  // connect to the bus and check for errors
  conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
  if (dbus_error_is_set(&err)) {
    std::cerr << "Connection Error " << err.message << "\n";
    dbus_error_free(&err);
  }
  if (NULL == conn) {
    std::cerr << "Connection Null\n";
    exit(1);
  }

  // request our name on the bus and check for errors
  ret = dbus_bus_request_name(conn, "org.openprinting.PrintDialog",  DBUS_NAME_FLAG_DO_NOT_QUEUE, &err);
  if (dbus_error_is_set(&err)) {
    std::cerr << "Name Error " << err.message << "\n";
    dbus_error_free(&err);
  }
  if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
    std::cerr << "Not Primary Owner " << ret << "\n";
    exit(1);
  }

  Manager manager;

  // loop, testing for new messages
  while (true) {
    // non blocking read of the next available message
    dbus_connection_read_write(conn, 0);
    msg = dbus_connection_pop_message(conn);

    // loop again if we haven't got a message
    if (NULL == msg) {
      sleep(1);
      continue;
    }

    std::cerr << "Path: " << dbus_message_get_path(msg) << "\n";
    std::cerr << "Interface: " << dbus_message_get_interface(msg) << "\n";

    // check this is a method call for the right interface & method

    manager.handleMessage(msg, conn);
    for(PrintDialog d : manager.dialogs)
      d.handleMessage(msg);

    if (dbus_message_is_method_call(msg, "org.freedesktop.DBus.Introspectable", "Introspect") &&
        dbus_message_has_path(msg, "/")) {
      DBusMessage* reply;
      DBusMessageIter args;
      dbus_uint32_t serial = 0;
      reply = dbus_message_new_method_return(msg);
      if (reply == NULL) {
        std::cerr << "Unable to allocate memory for reply\n";
        exit(0);
      }
      dbus_message_iter_init_append(reply, &args);

      std::string file_content = read_xml("org.openprinting.printdialog.xml");
      std::stringstream ss;
      ss << file_content << "\n";
      for(PrintDialog d : manager.dialogs)
        ss << "<node name=\"org/openprinting/PrintDialog/" << d.pathKey << "\"/>\n";
      ss << "</node>\n";
      std::string finalXML = ss.str();
      if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &finalXML)) {
        std::cerr << "Out Of Memory!\n";
        exit(1);
      }

      if (!dbus_connection_send(conn, reply, &serial)) {
        std::cerr << "Out Of Memory!\n";
        exit(1);
      }
      dbus_connection_flush(conn);

      // free the reply
      dbus_message_unref(reply);
    }

    // free the message
    dbus_message_unref(msg);
  }

  // close the connection
  dbus_connection_close(conn);
}

int main() {
  listen();
  return 0;
}
