#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>
#include <vector>

#include "introspect.hpp"

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
    std::cerr << "Unable to allocate memory for reply\n";
    exit(0);
  }
  dbus_message_iter_init_append(reply, &args);

  std::string file_content = read_xml(xml);

  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &file_content)) {
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
