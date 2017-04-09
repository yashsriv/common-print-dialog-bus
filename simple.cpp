#include <dbus/dbus.h>
#include <fstream>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <streambuf>
#include <string>
#include <unistd.h>
#include <vector>

enum dialog_t {
  qt,
  gtk
};

std::string read_xml(std::string xml) {

  std::ifstream xml_file(xml);
  std::string file_contents;

  xml_file.seekg(0, std::ios::end);
  file_contents.reserve(xml_file.tellg());
  xml_file.seekg(0, std::ios::beg);

  file_contents.assign((std::istreambuf_iterator<char>(xml_file)),
                       std::istreambuf_iterator<char>());

  return file_contents;

}

void introspect(DBusMessage* msg, DBusConnection* conn, std::string xml) {
  DBusMessage* reply;
  DBusMessageIter args;
  dbus_uint32_t serial = 0;
  reply = dbus_message_new_method_return(msg);
  if (reply == NULL) {
    fprintf(stderr, "Unable to allocate memory for reply\n");
    exit(0);
  }
  dbus_message_iter_init_append(reply, &args);

  std::string file_content = read_xml(xml);

  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &file_content)) {
    fprintf(stderr, "Out Of Memory!\n");
    exit(1);
  }

  if (!dbus_connection_send(conn, reply, &serial)) {
    fprintf(stderr, "Out Of Memory!\n");
    exit(1);
  }
  dbus_connection_flush(conn);

  // free the reply
  dbus_message_unref(reply);

}

void create_print_dialog(DBusMessage* msg, DBusConnection* conn) {
  DBusError err;
  DBusMessage* reply;
  DBusMessageIter args;
  dbus_uint32_t serial = 0;

  enum dialog_t dialog;

  printf("Creating PrintDialog");
  dbus_error_init(&err);

  if (dbus_bus_name_has_owner(conn, "org.gnome.Shell", &err)) {
    dialog = gtk;
  } else if (dbus_bus_name_has_owner(conn, "com.canonical.Unity", &err)) {
    dialog = gtk;
  } else {
    dialog = qt;
  }

  printf("Creating Dialog. %d", dialog);

  // create a reply from the message
  reply = dbus_message_new_method_return(msg);

  // add the arguments to the reply
  dbus_message_iter_init_append(reply, &args);
  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_OBJECT_PATH, "/org/openprinting/PrintDialog/Test")) {
    fprintf(stderr, "Out Of Memory!\n");
    exit(1);
  }

  // send the reply && flush the connection
  if (!dbus_connection_send(conn, reply, &serial)) {
    fprintf(stderr, "Out Of Memory!\n");
    exit(1);
  }
  dbus_connection_flush(conn);

  // free the reply
  dbus_message_unref(reply);
}


void listen() {
  DBusMessage* msg;
  DBusConnection* conn;
  DBusError err;

  int ret;

  printf("Listening for method calls\n");

  // initialise the error
  dbus_error_init(&err);

  // connect to the bus and check for errors
  conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
  if (dbus_error_is_set(&err)) {
    fprintf(stderr, "Connection Error (%s)\n", err.message);
    dbus_error_free(&err);
  }
  if (NULL == conn) {
    fprintf(stderr, "Connection Null\n");
    exit(1);
  }

  // request our name on the bus and check for errors
  ret = dbus_bus_request_name(conn, "org.openprinting.PrintDialog",  DBUS_NAME_FLAG_DO_NOT_QUEUE, &err);
  if (dbus_error_is_set(&err)) {
    fprintf(stderr, "Name Error (%s)\n", err.message);
    dbus_error_free(&err);
  }
  if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
    fprintf(stderr, "Not Primary Owner (%d)\n", ret);
    exit(1);
  }

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

    fprintf(stderr, "Path: %s\n", dbus_message_get_path(msg));

    // check this is a method call for the right interface & method
    if (dbus_message_is_method_call(msg, "org.freedesktop.DBus.Introspectable", "Introspect") &&
        dbus_message_has_path(msg, "/org/openprinting/PrintDialog/Manager"))
      introspect(msg, conn, "org.openprinting.printdialog.manager.xml");

    else if (dbus_message_is_method_call(msg, "org.openprinting.PrintDialog.Manager", "CreatePrintDialog") &&
        dbus_message_has_path(msg, "/org/openprinting/PrintDialog/Manager"))
      create_print_dialog(msg, conn);

    else if (dbus_message_is_method_call(msg, "org.freedesktop.DBus.Introspectable", "Introspect"))
      introspect(msg, conn, "org.openprinting.printdialog.xml");

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
