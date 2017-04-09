#ifndef PRINT_DIALOG_H
#define PRINT_DIALOG_H

#include <dbus/dbus.h>
#include <sys/socket.h>

#include <utility>
#include <vector>

#include "introspect.hpp"

struct Choice {
  std::string name;
  std::string text;
  // Optional Image
  std::vector<std::uint8_t> icon;
};

struct Option {
  std::string name;
  std::string text;
  std::string type;
  std::vector<Choice> choices;
  void* default_value;
  void* minimum_value;
  void* maximum_value;
  std::string regex;
  std::vector<std::string> tags;
  std::vector<std::string> optionHints;
  std::vector<std::uint8_t> icon;
};

class PrintDialog {
private:
  // DBus Connection for emitting signals
  DBusConnection* conn;

  // Socket Specific variables for receiving from socket
  int sock;
  sockaddr * sockname;
  socklen_t socklen;

  // Preview Document Info
  bool isPreview;
  unsigned int firstpage;
  bool canrequestmore;

  // Document Info
  std::string name;
  int pagecnt;
  unsigned int size;

  std::vector<Option> options;

  void show();
  void setDocumentName(std::string);
  void setNumberOfPages(int);
  void setDocumentSize(unsigned int);
  void requestDocumentDataSocket(DBusMessage*);
  void requestPreviewDataSocket(DBusMessage*);
  void addOption(DBusMessage*);
  void setOptionIcon(std::string, std::vector<std::uint8_t>);
  void setChoiceIcon(std::string, std::string, std::vector<std::uint8_t>);
  void getOptionValue(DBusMessage*);
  void setOptionValue(DBusMessage*);
  void addPreset(std::string, std::string);
public:
  std::string pathKey;
  PrintDialog(DBusConnection*, std::string);
  void OptionChanged(std::string, void*);
  void Cancelled();
  void ReadyForDocumentData();
  void NeedPreviewPage(int);
  void handleMessage(DBusMessage*);
};

#endif
