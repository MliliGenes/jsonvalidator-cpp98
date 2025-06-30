#include "utils.hpp"
#include <cstring>
#include <dirent.h>
#include <sstream>
#include <string>
#include <unistd.h>

void split(std::vector<std::string> &buff, const std::string &s, char deli) {
  std::string token;
  std::istringstream is(s);
  while (std::getline(is, token, deli))
    buff.push_back(token);
}

std::string perm_to_string(int perm) {
  std::string result;
  result += (perm & 4) ? 'r' : '-';
  result += (perm & 2) ? 'w' : '-';
  result += (perm & 1) ? 'x' : '-';

  return result;
}

FileInfo::FileInfo(std::string path) : path(path) {
  status = stat(path.c_str(), &sb);
}

bool FileInfo::isDir() { return status == 0 && (sb.st_mode & S_IFDIR); }

bool FileInfo::isFile() { return status == 0 && (sb.st_mode & S_IFREG); }

bool FileInfo::isExists() { return isFile() || isDir(); }

bool FileInfo::isReadable() { return hasPermissions(1, 0, 0); }

bool FileInfo::isWriteable() { return hasPermissions(0, 1, 0); }

bool FileInfo::isExecutable() { return hasPermissions(0, 0, 1); }

bool FileInfo::hasPermissions(bool read, bool write, bool exec) {
  int mode = 0;
  if (read)
    mode |= R_OK;
  if (write)
    mode |= W_OK;
  if (exec)
    mode |= X_OK;

  return access(path.c_str(), mode) == 0;
}

bool FileInfo::remove_directory(const std::string &path) {
  DIR *dir = opendir(path.c_str());
  if (!dir)
    return false;

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    std::string entry_path = path + "/" + entry->d_name;

    struct stat statbuf;
    if (stat(entry_path.c_str(), &statbuf) == 0) {
      if (S_ISDIR(statbuf.st_mode)) {
        if (!remove_directory(entry_path)) {
          closedir(dir);
          return false;
        }
      } else {
        if (unlink(entry_path.c_str()) != 0) {
          closedir(dir);
          return false;
        }
      }
    }
  }

  closedir(dir);
  return rmdir(path.c_str()) == 0;
}
