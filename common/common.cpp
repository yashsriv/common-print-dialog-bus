#include <fstream>
#include <string>

#include "common.hpp"

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
