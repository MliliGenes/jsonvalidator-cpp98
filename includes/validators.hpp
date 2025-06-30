#pragma once

#include "JsonTypes.hpp"
#include "utils.hpp"
#include <cstdlib>

bool isEmpty_(const JsonString &v);
bool isDigit_(const JsonString &v);
bool isPort_(const JsonString &v);
bool isIpv4_(const JsonString &v);
bool isValidPath_(const JsonString &v);
bool isValidDir_(const JsonString &v);
bool isValidFile_(const JsonString &v);

struct IChecker {
  virtual bool operator()(const JsonString &s) const = 0;
  virtual IChecker *clone() const = 0;
  virtual ~IChecker() {}
};

struct isStartWith_ : public IChecker {
  char c;
  isStartWith_(char c) : c(c) {}

  bool operator()(const JsonString &str) const {
    const std::string &value = str.value;
    if (value.empty())
      return false;
    if (value[0] == c)
      return true;
    return false;
  }
  IChecker *clone() const { return new isStartWith_(*this); }
};

struct isEqual_ : public IChecker {
  std::string value;
  isEqual_(const std::string &v) : value(v) {}

  bool operator()(const JsonString &str) const { return str.value == value; }
  IChecker *clone() const { return new isEqual_(*this); }
};

struct isFileWithPermissions_ : public IChecker {
  int permissions;
  isFileWithPermissions_(int perms) : permissions(perms) {}
  bool operator()(const JsonString &str) const {
    const std::string &value = str.value;
    if (value.empty())
      return false;
    FileInfo file(value);
    if (!file.isExists() || !file.isFile())
      return false;
    bool read = (permissions & 4) != 0;
    bool write = (permissions & 2) != 0;
    bool exec = (permissions & 1) != 0;
    if (file.hasPermissions(read, write, exec)) {
      return true;
    }
    return false;
  }
  IChecker *clone() const { return new isFileWithPermissions_(*this); }
};

struct isDirWithPermissions_ : public IChecker {
  int permissions;
  isDirWithPermissions_(int perms) : permissions(perms) {}
  bool operator()(const JsonString &str) const {
    const std::string &value = str.value;
    if (value.empty())
      return false;
    FileInfo file(value);
    if (!file.isExists() || !file.isDir())
      return false;
    bool read = (permissions & 4) != 0;
    bool write = (permissions & 2) != 0;
    bool exec = (permissions & 1) != 0;
    if (file.hasPermissions(read, write, exec)) {
      return true;
    }
    return false;
  }
  IChecker *clone() const { return new isDirWithPermissions_(*this); }
};
