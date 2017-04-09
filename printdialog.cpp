#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#include "printdialog.hpp"

void listenOnSocket(int sock, sockaddr * sockname, socklen_t * socklen, std::string path) {
  listen(sock, 3);
  int dataSocket= accept(sock, sockname, socklen);
  char buffer[256];

  std::stringstream ss;

  ss << "/tmp/.cpd/files/" << path;
  std::ofstream tmpFile(ss.str(), std::fstream::out | std::fstream::binary | std::fstream::trunc);

  int resultSize = 0;
  bool receive = true;
  while (receive) {
    resultSize = recv(dataSocket, buffer, sizeof(buffer), 0);

    if (resultSize == 0) {
      receive = false;
    }

    for (int i = 0; i < resultSize && receive; i++) {
      tmpFile << buffer[i];
      if (buffer[i] == EOF) {
        receive = false;
      }
    }
  }

  tmpFile.close();

  close(sock);
}

PrintDialog::PrintDialog(DBusConnection* conn, std::string pathKey) {
  this -> conn = conn;
  this -> pathKey = pathKey;
  this -> name = pathKey;
  this -> pagecnt = 0;
  this -> sock = -1;
}

void PrintDialog::show() {
  std::cerr << "Showing Dialog Now";
}

void PrintDialog::setDocumentName(std::string name) {
  this -> name = name;
}

void PrintDialog::setNumberOfPages(int pagecnt) {
  this -> pagecnt = pagecnt;
}

void PrintDialog::setDocumentSize(unsigned int size) {
  this -> size = size;
}

void PrintDialog::requestDocumentDataSocket(DBusMessage* msg) {

  sockaddr_un name;
  if (this -> sock >= 0) {
    name.sun_family = AF_LOCAL;
    std::stringstream ss;
    ss << "/tmp/.cpd/" << pathKey;
    strncpy(name.sun_path, ss.str().c_str(), sizeof(name.sun_path));
    unlink(name.sun_path);
    close(this -> sock);

  }
  int sock;
  size_t size;

  /* Create the socket. */
  sock = socket(PF_LOCAL, SOCK_STREAM, 0);

  if (sock < 0) {
    std::cerr << "Unable to create socket\n";
    exit (1);
  }

  name.sun_family = AF_LOCAL;

  std::stringstream ss;

  ss << "/tmp/.cpd/" << pathKey;
  strncpy(name.sun_path, ss.str().c_str(), sizeof(name.sun_path));
  size = SUN_LEN (&name);

  if (bind (sock, (sockaddr *) &name, size) < 0) {
    std::cerr << "Unable to bind socket\n";
    exit(1);
  }
  this -> sockname = (sockaddr *) &name;
  this -> socklen = size;

  DBusMessage* reply;
  DBusMessageIter args;
  dbus_uint32_t serial = 0;
  reply = dbus_message_new_method_return(msg);
  if (reply == NULL) {
    std::cerr << "Unable to allocate memory for reply\n";
    exit(1);
  }
  dbus_message_iter_init_append(reply, &args);

  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &name.sun_path)) {
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

  this -> sock = sock;
  this -> isPreview = false;

  // std::thread listener(listenOnSocket, sock, &name, &size, this -> pathKey);

}

void PrintDialog::requestPreviewDataSocket(DBusMessage* msg) {

  DBusMessageIter args;
  if (!dbus_message_iter_init(msg, &args))
    std::cerr << "Message has no arguments!\n";
  if (DBUS_TYPE_UINT32  != dbus_message_iter_get_arg_type(&args))
    std::cerr << "Argument is not uint32!\n";
  else
    dbus_message_iter_get_basic(&args, &firstpage);
  dbus_message_iter_next (&args);
  if (DBUS_TYPE_BOOLEAN  != dbus_message_iter_get_arg_type(&args))
    std::cerr << "Argument is not boolean!\n";
  else
    dbus_message_iter_get_basic(&args, &canrequestmore);

  this -> requestDocumentDataSocket(msg);
  this -> isPreview = true;

}

void PrintDialog::Cancelled() {

  std::stringstream ss;
  DBusMessage* signal;
  dbus_uint32_t serial = 0;

  ss << "/org/openprinting/PrintDialog/" << pathKey;

  signal = dbus_message_new_signal(ss.str().c_str(), "org.openprinting.PrintDialog.PrintDialog", "Cancelled");

  if (!dbus_connection_send(this -> conn, signal, &serial)) {
    std::cerr << "Out Of Memory!\n";
    exit(1);
  }
  dbus_connection_flush(this -> conn);

  // free the reply
  dbus_message_unref(signal);

}

void PrintDialog::ReadyForDocumentData() {

  std::stringstream ss;
  DBusMessage* signal;
  dbus_uint32_t serial = 0;

  ss << "/org/openprinting/PrintDialog/" << pathKey;

  signal = dbus_message_new_signal(ss.str().c_str(), "org.openprinting.PrintDialog.PrintDialog", "ReadyForDocumentData");

  if (!dbus_connection_send(this -> conn, signal, &serial)) {
    std::cerr << "Out Of Memory!\n";
    exit(1);
  }
  dbus_connection_flush(this -> conn);

  // free the reply
  dbus_message_unref(signal);

}

void PrintDialog::NeedPreviewPage(int page) {

  std::stringstream ss;
  DBusMessage* signal;
  DBusMessageIter args;
  dbus_uint32_t serial = 0;

  ss << "/org/openprinting/PrintDialog/" << pathKey;

  signal = dbus_message_new_signal(ss.str().c_str(), "org.openprinting.PrintDialog.PrintDialog", "ReadyForDocumentData");

  dbus_message_iter_init_append(signal, &args);
  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &page)) {
    std::cerr << "Out Of Memory!\n";
    exit(1);
  }

  if (!dbus_connection_send(this -> conn, signal, &serial)) {
    std::cerr << "Out Of Memory!\n";
    exit(1);
  }
  dbus_connection_flush(this -> conn);

  // free the reply
  dbus_message_unref(signal);

}

void PrintDialog::handleMessage(DBusMessage* msg) {
  std::stringstream ss;
  ss << "/org/openprinting/PrintDialog/" << this -> pathKey;
  std::string path = ss.str();
  if (dbus_message_is_method_call(msg, "org.freedesktop.DBus.Introspectable", "Introspect") &&
      dbus_message_has_path(msg, path.c_str()))
    introspect(msg, conn, "org.openprinting.printdialog.printdialog.xml");
}
