#pragma once

#include "AJsonValue.hpp"

#include <cstdlib>
#include <map>
#include <string>
#include <vector>

class JsonBool : public AJsonValue {
public:
  JsonBool();
  bool value;
  JsonBool(std::string);
  JsonBool(bool);
  JsonBool(const JsonBool &);
  AJsonValue *clone() const;
  bool isEqual(const AJsonValue &other) const;
  bool isEqual(AJsonValue &other);
};

class JsonNumber : public AJsonValue {
public:
  JsonNumber();
  long value;
  JsonNumber(std::string);
  JsonNumber(long);
  JsonNumber(const JsonNumber &);
  AJsonValue *clone() const;
  bool isEqual(const AJsonValue &other) const;
  bool isEqual(AJsonValue &other);
};

class JsonDouble : public AJsonValue {
public:
  JsonDouble();
  double value;
  JsonDouble(std::string);
  JsonDouble(const JsonDouble &);
  AJsonValue *clone() const;
  bool isEqual(const AJsonValue &other) const;
  bool isEqual(AJsonValue &other);
};

class JsonNull : public AJsonValue {
public:
  JsonNull();
  void *value;
  JsonNull(const JsonNull &);
  AJsonValue *clone() const;
  bool isEqual(const AJsonValue &other) const;
  bool isEqual(AJsonValue &other);
};

class JsonString : public AJsonValue {
public:
  std::string value;

  JsonString();
  JsonString(std::string);
  JsonString(const JsonString &);
  AJsonValue *clone() const;
  bool isEqual(const AJsonValue &other) const;
  bool isEqual(AJsonValue &other);
};

class JsonObject : public AJsonValue {
public:
  typedef std::map<std::string, AJsonValue *> container;
  typedef container::iterator iterator;
  typedef container::const_iterator const_iterator;
  container members;

  JsonObject();
  JsonObject(const JsonObject &);
  void setProperty(const std::string &key, AJsonValue *value);
  void replaceProperty(const std::string &key, AJsonValue *value);
  size_t size() const;
  iterator begin();
  const_iterator begin() const;
  const_iterator find(std::string &) const;
  iterator end();
  const_iterator end() const;
  bool empty() const;
  AJsonValue &operator[](const std::string &);
  AJsonValue *clone() const;
  bool isEqual(const AJsonValue &other) const;
  bool isEqual(AJsonValue &other);
  ~JsonObject();
};

class JsonArray : public AJsonValue {
public:
  typedef std::vector<AJsonValue *> container;
  typedef container::iterator iterator;
  typedef container::const_iterator const_iterator;

  JsonArray();
  JsonArray(const JsonArray &);
  container elements;
  size_t size() const;
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  bool empty() const;
  AJsonValue &operator[](const unsigned long &);
  AJsonValue *clone() const;
  AJsonValue &find(std::string);
  bool isEqual(const AJsonValue &other) const;
  bool isEqual(AJsonValue &other);
  ~JsonArray();
};
