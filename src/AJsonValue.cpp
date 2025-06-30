#include "AJsonValue.hpp"
#include "JsonTypes.hpp"
#include "utils.hpp"
#include <iostream>
#include <stdexcept>

AJsonValue::~AJsonValue() {}

json_type AJsonValue::getType() const {
  if (dynamic_cast<const JsonObject *>(this))
    return OBJECT;
  else if (dynamic_cast<const JsonArray *>(this))
    return ARRAY;
  else if (dynamic_cast<const JsonString *>(this))
    return STRING;
  else if (dynamic_cast<const JsonBool *>(this))
    return BOOLEAN;
  else if (dynamic_cast<const JsonNumber *>(this))
    return NUMBER;
  else if (dynamic_cast<const JsonDouble *>(this))
    return DOUBLE;
  return NIL;
}

std::string match_json_name(json_type type) {
  if (type == BOOLEAN)
    return "BOOLEAN";
  else if (type == NUMBER)
    return "NUMBER";
  else if (type == DOUBLE)
    return "DOUBLE";
  else if (type == STRING)
    return "STRING";
  else if (type == OBJECT)
    return "OBJECT";
  else if (type == ARRAY)
    return "ARRAY";
  return "NULL";
}

double AJsonValue::asDouble() const {
  const JsonDouble *json = dynamic_cast<const JsonDouble *>(this);
  if (!json)
    return 0.0;
  return json->value;
}

long AJsonValue::asNumber() const {
  const JsonNumber *json = dynamic_cast<const JsonNumber *>(this);
  if (!json)
    return 0;
  return json->value;
}

std::string AJsonValue::asString() const {
  const JsonString *json = dynamic_cast<const JsonString *>(this);
  if (!json)
    return "";
  return json->value;
}

bool AJsonValue::asBool() const {
  const JsonBool *json = dynamic_cast<const JsonBool *>(this);
  if (!json)
    return false;
  return json->value;
}
JsonObject *AJsonValue::asObject() const {
  const JsonObject *json = dynamic_cast<const JsonObject *>(this);
  if (!json)
    return NULL;
  return const_cast<JsonObject *>(json);
}

JsonObject &AJsonValue::asRefObject() const {
  const JsonObject *json = dynamic_cast<const JsonObject *>(this);
  return const_cast<JsonObject &>(*json);
}

JsonArray &AJsonValue::asRefArray() const {
  const JsonArray *json = dynamic_cast<const JsonArray *>(this);
  return const_cast<JsonArray &>(*json);
}

JsonArray *AJsonValue::asArray() const {
  const JsonArray *json = dynamic_cast<const JsonArray *>(this);
  if (!json)
    return NULL;
  return const_cast<JsonArray *>(json);
}

AJsonValue &AJsonValue::operator[](const std::string &item) {
  static JsonNull null;
  JsonObject *obj = dynamic_cast<JsonObject *>(this);
  if (!obj)
    return null;
  JsonObject::iterator it = obj->members.find(item);
  if (it == (*obj).members.end())
    return null;
  return *(it->second);
}

AJsonValue &AJsonValue::operator[](const unsigned long &idx) {
  JsonArray *arr = dynamic_cast<JsonArray *>(this);
  static JsonNull null;
  if (!arr || idx >= arr->size())
    return null;
  return (*arr)[idx];
}

AJsonValue &AJsonValue::at(const std::string &item) {
  JsonObject *obj = dynamic_cast<JsonObject *>(this);
  if (!obj)
    throw std::runtime_error("Not an object");
  JsonObject::iterator it = obj->members.find(item);
  if (it == obj->members.end())
    throw std::runtime_error("Key '" + item + "' not found");
  return *(it->second);
}

AJsonValue &AJsonValue::at(const unsigned long &idx) {
  JsonArray *arr = dynamic_cast<JsonArray *>(this);
  if (!arr)
    throw std::runtime_error("Not an array");
  if (idx >= arr->size())
    throw std::runtime_error("Index [" + to_string(idx) + "] out of range");
  return (*arr)[idx];
}

AJsonValue &AJsonValue::operator[](const std::string &item) const {
  const JsonObject *obj = dynamic_cast<const JsonObject *>(this);
  static JsonNull null;
  if (!obj)
    return null;
  JsonObject::const_iterator it = obj->members.find(item);
  if (!obj || it == (*obj).members.end())
    return null;
  return *(it->second);
}

AJsonValue &AJsonValue::operator[](const unsigned long &idx) const {
  const JsonArray *arr = dynamic_cast<const JsonArray *>(this);
  static JsonNull null;
  if (!arr || idx >= arr->size())
    return null;
  return (*arr).at(idx);
}

AJsonValue &AJsonValue::at(const std::string &item) const {
  const JsonObject *obj = dynamic_cast<const JsonObject *>(this);
  if (!obj)
    throw std::runtime_error("Not an object");
  JsonObject::const_iterator it = obj->members.find(item);
  if (it == obj->members.end())
    throw std::runtime_error("Key '" + item + "' not found");
  return *(it->second);
}

AJsonValue &AJsonValue::at(const unsigned long &idx) const {
  const JsonArray *arr = dynamic_cast<const JsonArray *>(this);
  if (!arr)
    throw std::runtime_error("Not an array");
  if (idx >= arr->size())
    throw std::runtime_error("Index [" + to_string(idx) + "] out of range");
  return (*arr).at(idx);
}

bool AJsonValue::isArray() const {
  return (dynamic_cast<const JsonArray *>(this));
}
bool AJsonValue::isObject() const {
  return (dynamic_cast<const JsonObject *>(this));
}
bool AJsonValue::isNumber() const {
  return (dynamic_cast<const JsonNumber *>(this));
}
bool AJsonValue::isString() const {
  return (dynamic_cast<const JsonString *>(this));
}
bool AJsonValue::isDouble() const {
  return (dynamic_cast<const JsonDouble *>(this));
}
bool AJsonValue::isNull() const {
  return (dynamic_cast<const JsonNull *>(this));
}
bool AJsonValue::isEmpty() const {
  return dynamic_cast<const void *>(this) == NULL;
}
bool AJsonValue::isBool() const {
  return (dynamic_cast<const JsonBool *>(this));
}

AJsonValue::operator bool_type() const {
  return getType() == NIL ? 0 : &AJsonValue::dummy;
}

void AJsonValue::dummy() const {}

bool AJsonValue::operator==(const AJsonValue &obj) {
  if (getType() != obj.getType())
    return false;
  return isEqual(obj);
}

bool AJsonValue::operator==(AJsonValue &obj) {
  if (getType() != obj.getType())
    return false;
  return isEqual(obj);
}

std::ostream &printJson(std::ostream &os, const AJsonValue &v,
                        unsigned indent) {
  const std::string indentStr(indent * 2, ' ');

  if (v.isNull() || v.isEmpty()) {
    os << "(null)";
  } else if (v.isString()) {
    os << '"' << v.asString() << '"';
  } else if (v.isNumber()) {
    os << v.asNumber();
  } else if (v.isDouble()) {
    os << v.asDouble();
  } else if (v.isBool()) {
    os << (v.asBool() ? "true" : "false");
  } else if (v.isObject()) {
    JsonObject &obj = *v.asObject();
    os << "{\n";
    JsonObject::const_iterator it = obj.begin();
    while (it != obj.end()) {
      os << std::string((indent + 1) * 2, ' ') << '"' << it->first << "\": ";
      printJson(os, *it->second, indent + 1);
      ++it;
      if (it != obj.end())
        os << ",";
      os << "\n";
    }
    os << indentStr << "}";
  } else if (v.isArray()) {
    JsonArray &arr = *v.asArray();
    os << "[\n";
    for (size_t i = 0; i < arr.size(); ++i) {
      os << std::string((indent + 1) * 2, ' ');
      printJson(os, arr[i], indent + 1);
      if (i + 1 < arr.size())
        os << ",";
      os << "\n";
    }
    os << indentStr << "]";
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const AJsonValue &v) {
  return printJson(os, v);
}
