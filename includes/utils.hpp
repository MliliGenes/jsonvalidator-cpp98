#pragma once

#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

void split(std::vector<std::string> &buff, const std::string &s, char deli);
std::string perm_to_string(int);

#ifndef SIZE_T_MAX
#define SIZE_T_MAX ((size_t)(-1))
#endif

template <typename T> std::string to_string(const T &value) {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

class FileInfo {
private:
  int status;
  struct stat sb;
  std::string path;

public:
  FileInfo(std::string);
  bool isFile();
  bool isDir();
  bool isExists();
  bool isReadable();
  bool isWriteable();
  bool isExecutable();
  bool hasPermissions(bool read, bool write, bool exec);
  static bool remove_directory(const std::string &path);
};
