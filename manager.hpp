#ifndef MANAGER_H
#define MANAGER_H

#include <dbus/dbus.h>
#include "printdialog.hpp"
#include <vector>

class Manager {
private:
  unsigned int uid;
  void createPrintDialog(DBusMessage*, DBusConnection*);
public:
  std::vector<PrintDialog> dialogs;
  Manager();
  void handleMessage(DBusMessage*, DBusConnection*);
};

#endif
