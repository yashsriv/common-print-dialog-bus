#include <iostream>
#include <sstream>

#include "introspect.hpp"
#include "manager.hpp"

Manager::Manager() {
  this -> uid = 0;
}

void Manager::createPrintDialog(DBusMessage* msg, DBusConnection* conn) {
  DBusError err;
  DBusMessage* reply;
  DBusMessageIter args;
  dbus_uint32_t serial = 0;

  dbus_error_init(&err);

  std::string sender = dbus_message_get_sender(msg);

  std::stringstream ss;

  ss << "/org/openprinting/PrintDialog/" << uid;

  PrintDialog dialog(conn, std::to_string(uid));
  this -> dialogs.push_back(dialog);
  uid++;

  // create a reply from the message
  reply = dbus_message_new_method_return(msg);

  std::string objectPath = ss.str();

  // add the arguments to the reply
  dbus_message_iter_init_append(reply, &args);
  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_OBJECT_PATH, &objectPath)) {
    std::cerr << "Out Of Memory!\n";
    exit(1);
  }

  // send the reply && flush the connection
  if (!dbus_connection_send(conn, reply, &serial)) {
    std::cerr << "Out Of Memory!\n";
    exit(1);
  }
  dbus_connection_flush(conn);

  // free the reply
  dbus_message_unref(reply);

}

void Manager::handleMessage(DBusMessage* msg, DBusConnection* conn) {
  if (dbus_message_has_path(msg, "/org/openprinting/PrintDialog/Manager")) {
    if (dbus_message_is_method_call(msg, "org.openprinting.PrintDialog.Manager", "CreatePrintDialog"))
      this -> createPrintDialog(msg, conn);
    else if (dbus_message_is_method_call(msg, "org.freedesktop.DBus.Introspectable", "Introspect"))
      introspect(msg, conn, "org.openprinting.printdialog.manager.xml");
  }
}
