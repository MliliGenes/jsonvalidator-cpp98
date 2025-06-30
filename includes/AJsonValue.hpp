#pragma once

#include <ostream>
#include <string>
typedef enum json_type {
  BOOLEAN,
  NUMBER,
  DOUBLE,
  NIL,
  STRING,
  OBJECT,
  ARRAY,
  UNDEFINED
} json_type;

class JsonObject;
class JsonArray;

class AJsonValue {
public:
  typedef void (AJsonValue::*bool_type)() const;
  virtual ~AJsonValue();
  json_type getType() const;
  void print() const;
  double asDouble() const;
  long asNumber() const;
  std::string asString() const;
  bool asBool() const;
  JsonObject *asObject() const;
  JsonObject &asRefObject() const;
  JsonArray *asArray() const;
  JsonArray &asRefArray() const;
  bool isArray() const;
  bool isObject() const;
  bool isNumber() const;
  bool isString() const;
  bool isEmpty() const;
  bool isDouble() const;
  bool isNull() const;
  bool isBool() const;
  operator bool_type() const;
  void dummy() const;
  AJsonValue &operator[](const std::string &);
  AJsonValue &operator[](const unsigned long &);
  AJsonValue &at(const std::string &);
  AJsonValue &at(const unsigned long &);
  AJsonValue &operator[](const std::string &) const;
  AJsonValue &operator[](const unsigned long &) const;
  AJsonValue &at(const std::string &) const;
  AJsonValue &at(const unsigned long &) const;
  virtual AJsonValue *clone() const = 0;
  bool operator==(const AJsonValue &);
  bool operator==(AJsonValue &);
  virtual bool isEqual(const AJsonValue &other) const = 0;
  virtual bool isEqual(AJsonValue &other) = 0;

  // class TypeErrorException : public std::exception {
  // public:
  //   const char *what() const throw();
  // };

  // class OutOfRangeException : public std::exception {
  // public:
  //   const char *what() const throw();
  // };

  // class ElementNotExistException : public std::exception {
  // public:
  //   const char *what() const throw();
  // };
};

std::string match_json_name(json_type type);

std::ostream &printJson(std::ostream &os, const AJsonValue &v,
                        unsigned indent = 0);
std::ostream &operator<<(std::ostream &, const AJsonValue &);
