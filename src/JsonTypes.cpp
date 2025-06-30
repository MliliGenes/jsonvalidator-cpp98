#include "JsonTypes.hpp"
#include "AJsonValue.hpp"

JsonString::JsonString() {}
JsonString::JsonString(std::string value) : value(value) {}
JsonString::JsonString(const JsonString &obj) : value(obj.value) {}

AJsonValue *JsonString::clone() const { return new JsonString(*this); }

bool JsonString::isEqual(const AJsonValue &other) const {
  const JsonString &otherValue = dynamic_cast<const JsonString &>(other);
  return value == otherValue.value;
}

bool JsonString::isEqual(AJsonValue &other) {
  JsonString &otherValue = dynamic_cast<JsonString &>(other);
  return value == otherValue.value;
}

JsonNumber::JsonNumber() : value(0) {}
JsonNumber::JsonNumber(long value) : value(value) {}
JsonNumber::JsonNumber(std::string value) : value(std::atol(value.c_str())) {}
JsonNumber::JsonNumber(const JsonNumber &obj) : value(obj.value) {}

AJsonValue *JsonNumber::clone() const { return new JsonNumber(*this); }

bool JsonNumber::isEqual(const AJsonValue &other) const {
  const JsonNumber &otherValue = dynamic_cast<const JsonNumber &>(other);
  return value == otherValue.value;
}

bool JsonNumber::isEqual(AJsonValue &other) {
  JsonNumber &otherValue = dynamic_cast<JsonNumber &>(other);
  return value == otherValue.value;
}

JsonDouble::JsonDouble() : value(0) {}
JsonDouble::JsonDouble(std::string value) : value(std::atof(value.c_str())) {}
JsonDouble::JsonDouble(const JsonDouble &obj) : value(obj.value) {}

AJsonValue *JsonDouble::clone() const { return new JsonDouble(*this); }

bool JsonDouble::isEqual(const AJsonValue &other) const {
  const JsonDouble &otherValue = dynamic_cast<const JsonDouble &>(other);
  return value == otherValue.value;
}

bool JsonDouble::isEqual(AJsonValue &other) {
  JsonDouble &otherValue = dynamic_cast<JsonDouble &>(other);
  return value == otherValue.value;
}

JsonNull::JsonNull() : value(NULL) {}
JsonNull::JsonNull(const JsonNull &obj) : value(obj.value) {}

AJsonValue *JsonNull::clone() const { return new JsonNull(*this); }

bool JsonNull::isEqual(const AJsonValue &other) const {
  const JsonNull &otherValue = dynamic_cast<const JsonNull &>(other);
  return value == otherValue.value;
}

bool JsonNull::isEqual(AJsonValue &other) {
  JsonNull &otherValue = dynamic_cast<JsonNull &>(other);
  return value == otherValue.value;
}

JsonBool::JsonBool() : value(0) {}
JsonBool::JsonBool(bool value) : value(value) {}
JsonBool::JsonBool(std::string value) : value(value == "true") {}
JsonBool::JsonBool(const JsonBool &obj) : value(obj.value) {}

AJsonValue *JsonBool::clone() const { return new JsonBool(*this); }

bool JsonBool::isEqual(const AJsonValue &other) const {
  const JsonBool &otherValue = dynamic_cast<const JsonBool &>(other);
  return value == otherValue.value;
}

bool JsonBool::isEqual(AJsonValue &other) {
  JsonBool &otherValue = dynamic_cast<JsonBool &>(other);
  return value == otherValue.value;
}

JsonObject::JsonObject() {}

AJsonValue &JsonObject::operator[](const std::string &item) {
  return *(this->members[item]);
}

JsonObject::JsonObject(const JsonObject &obj) {
  const_iterator it = obj.begin();
  for (; it != obj.end(); it++)
    members[it->first] = it->second->clone();
}

void JsonObject::setProperty(const std::string &key, AJsonValue *value) {
  std::map<std::string, AJsonValue *>::iterator it = members.find(key);
  if (it != members.end())
    delete it->second;
  members[key] = value;
}

void JsonObject::replaceProperty(const std::string &key, AJsonValue *value) {
  std::map<std::string, AJsonValue *>::iterator it = members.find(key);
  if (it != members.end()) {
    delete it->second;
    it->second = value;
  } else {
    members[key] = value;
  }
}

AJsonValue *JsonObject::clone() const { return new JsonObject(*this); }

size_t JsonObject::size() const { return members.size(); }
bool JsonObject::empty() const { return members.empty(); }
JsonObject::iterator JsonObject::begin() { return members.begin(); }
JsonObject::const_iterator JsonObject::begin() const { return members.begin(); }
JsonObject::iterator JsonObject::end() { return members.end(); }
JsonObject::const_iterator JsonObject::end() const { return members.end(); }

JsonObject::const_iterator JsonObject::find(std::string &key) const {
  return members.find(key);
}

bool JsonObject::isEqual(const AJsonValue &other) const {
  const JsonObject &otherObj = dynamic_cast<const JsonObject &>(other);

  if (members.size() != otherObj.members.size())
    return false;

  const_iterator it = members.begin();
  const_iterator otherIt = otherObj.members.begin();

  for (; it != members.end(); ++it, ++otherIt) {
    if (it->first != otherIt->first)
      return false;
    if (it->second == otherIt->second)
      continue;
    if (!it->second || !otherIt->second || !(*it->second == *otherIt->second))
      return false;
  }
  return true;
}

bool JsonObject::isEqual(AJsonValue &other) {
  JsonObject &otherObj = dynamic_cast<JsonObject &>(other);

  if (members.size() != otherObj.members.size())
    return false;

  const_iterator it = members.begin();
  const_iterator otherIt = otherObj.members.begin();

  for (; it != members.end(); ++it, ++otherIt) {
    if (it->first != otherIt->first)
      return false;
    if (it->second == otherIt->second)
      continue;
    if (!it->second || !otherIt->second || !(*it->second == *otherIt->second))
      return false;
  }
  return true;
}

JsonObject::~JsonObject() {
  container::iterator it = members.begin();
  for (; it != members.end(); it++) {
    delete it->second;
  }
}

JsonArray::JsonArray() {}

AJsonValue &JsonArray::operator[](const unsigned long &idx) {
  return *(this->elements[idx]);
}

AJsonValue *JsonArray::clone() const { return new JsonArray(*this); }

JsonArray::JsonArray(const JsonArray &obj) {
  const_iterator it = obj.begin();
  for (; it != obj.end(); it++)
    elements.push_back((*it)->clone());
}

size_t JsonArray::size() const { return elements.size(); }
bool JsonArray::empty() const { return elements.empty(); }
JsonArray::iterator JsonArray::begin() { return elements.begin(); }
JsonArray::const_iterator JsonArray::begin() const { return elements.begin(); }
JsonArray::iterator JsonArray::end() { return elements.end(); }
JsonArray::const_iterator JsonArray::end() const { return elements.end(); }

bool JsonArray::isEqual(const AJsonValue &other) const {
  const JsonArray &otherArray = dynamic_cast<const JsonArray &>(other);

  if (elements.size() != otherArray.elements.size())
    return false;

  for (size_t i = 0; i < elements.size(); ++i) {
    AJsonValue *val1 = elements[i];
    AJsonValue *val2 = otherArray.elements[i];

    if ((val1 == NULL) != (val2 == NULL))
      return false;

    if (val1 && val2 && !(*val1 == *val2))
      return false;
  }
  return true;
}

bool JsonArray::isEqual(AJsonValue &other) {
  JsonArray &otherArray = dynamic_cast<JsonArray &>(other);

  if (elements.size() != otherArray.elements.size())
    return false;

  for (size_t i = 0; i < elements.size(); ++i) {
    AJsonValue *val1 = elements[i];
    AJsonValue *val2 = otherArray.elements[i];

    if ((val1 == NULL) != (val2 == NULL))
      return false;

    if (val1 && val2 && !(*val1 == *val2))
      return false;
  }
  return true;
}

AJsonValue &JsonArray::find(std::string item) {
  static JsonNull null;
  if (isEmpty())
    return null;
  for (size_t i = 0; i < elements.size(); ++i) {
    if (!elements[i])
      continue;
    if (*elements[i] == JsonString(item))
      return *elements[i];
  }
  return null;
}

JsonArray::~JsonArray() {
  for (unsigned long i = 0; i < elements.size(); i++) {
    delete elements[i];
  }
}
