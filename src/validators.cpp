#include "validators.hpp"
#include "utils.hpp"
#include <string>

bool isEmpty_(const JsonString &v) {
  const std::string &value = v.value;
  if (value.empty())
    return false;
  if (value.find_first_not_of(" \t\v\n\f\r") == std::string::npos)
    return false;
  return true;
}

bool isDigit_(const JsonString &v) {
  const std::string &value = v.value;
  if (value.empty())
    return false;
  if (value.find_first_not_of("0123456789") != std::string::npos)
    return false;
  return true;
}

bool isPort_(const JsonString &v) {
  const std::string &value = v.value;
  if (!isDigit_(v))
    return false;
  int port = atoi(value.c_str());
  if (port > 65535)
    return false;
  return true;
}

bool isIpv4_(const JsonString &v) {
  const std::string &value = v.value;
  if (value.empty() || value.length() > 15)
    return false;
  if (value.find_first_not_of("0123456789.") != std::string::npos)
    return false;
  std::vector<std::string> parts;
  split(parts, value, '.');
  if (parts.size() != 4)
    return false;
  for (unsigned i = 0; i < parts.size(); i++) {
    if (parts[i].empty() ||
        (parts[i].length() == 3 && parts[i].compare("255") > 0))
      return false;
  }
  return true;
}

bool isValidPath_(const JsonString &v) {
  const std::string &value = v.value;
  if (value.empty())
    return false;
  FileInfo file(value);
  if (!file.isExists())
    return false;
  return true;
}

bool isValidDir_(const JsonString &v) {
  const std::string &value = v.value;
  if (value.empty())
    return false;
  FileInfo file(value);
  if (!file.isExists() || !file.isDir())
    return false;
  return true;
}

bool isValidFile_(const JsonString &v) {
  const std::string &value = v.value;
  if (value.empty())
    return false;
  FileInfo file(value);
  if (!file.isExists() || !file.isFile())
    return false;
  return true;
}
