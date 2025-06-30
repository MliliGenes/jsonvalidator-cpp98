#pragma once

#include "AJsonValue.hpp"
#include "JsonTypes.hpp"
#include "validators.hpp"
#include <map>
#include <string>

class ValidationError {
public:
  std::string path;
  std::string msg;

  ValidationError(const std::string &, const std::string &);
};

class AJsonValidator {
public:
  typedef std::vector<ValidationError> ValidationErr;

protected:
  ValidationErr errors_;
  bool optional_;
  bool hasDefault_;
  json_type exceptedType_;
  AJsonValue *defaultValue_;

public:
  AJsonValidator();
  AJsonValidator(const AJsonValidator &);
  const ValidationErr &getErrors() const;
  void clearErrors();
  virtual ~AJsonValidator();
  virtual bool validate(const AJsonValue *, const std::string &path = "") = 0;
  virtual AJsonValidator *clone() const = 0;
  void set_optional();
  bool get_optional() const;
  bool isTypeCompatible(const AJsonValidator &, const AJsonValue *v) const;
  virtual AJsonValue *get_default() const;
  bool has_default() const;
  void set_default(AJsonValue *);
  virtual AJsonValue *applyDefaults(AJsonValue *);

protected:
  void addError(const std::string &, const std::string &);
};

class TypeValidator : public AJsonValidator {

public:
  TypeValidator(json_type);
  AJsonValidator *clone() const;
  bool validate(const AJsonValue *, const std::string &path = "");
};

class StringValidator;

class ObjectValidator : public AJsonValidator {
private:
  typedef std::map<std::string, AJsonValidator *> ValidatorMap;
  typedef ValidatorMap::iterator iterator;
  typedef ValidatorMap::const_iterator const_iterator;

  ValidatorMap properties_;
  bool allowAdditional_;
  bool matchMode_;
  bool emptyCheck;
  StringValidator *key_validator;
  AJsonValidator *val_validator;
  AJsonValidator *last_;

public:
  ObjectValidator();
  ObjectValidator(const ObjectValidator &);
  ObjectValidator &property(const std::string &name, const AJsonValidator &v);
  ObjectValidator &match(const StringValidator &, const AJsonValidator &);
  ObjectValidator &allowAdditional(bool);
  ObjectValidator &optional();
  ObjectValidator &notEmpty();
  bool validate(const AJsonValue *, const std::string &path = "");
  AJsonValidator *clone() const;
  ObjectValidator &withDefault(JsonObject &v);
  AJsonValue *applyDefaults(AJsonValue *);
  ~ObjectValidator();
};

class ArrayValidator : public AJsonValidator {
private:
  AJsonValidator *validator_;
  size_t min_;
  size_t max_;
  JsonArray *defaultValue_;

public:
  ArrayValidator();
  ArrayValidator(const ArrayValidator &);
  ArrayValidator &optional();
  bool validate(const AJsonValue *, const std::string &path = "");
  ArrayValidator &min(size_t);
  ArrayValidator &max(size_t);
  ArrayValidator &item(const AJsonValidator &v);
  AJsonValidator *clone() const;
  AJsonValue *get_default() const;
  ArrayValidator &withDefault(JsonArray &v);
  AJsonValue *applyDefaults(AJsonValue *);
  ~ArrayValidator();
};

class ORValidator : public AJsonValidator {
private:
  std::vector<AJsonValidator *> conditions_;
  std::string msg_;

public:
  ORValidator();
  ORValidator(const ORValidator &);
  ORValidator &addConditions(const AJsonValidator &v);
  ORValidator &withMsg(const std::string &msg);
  bool validate(const AJsonValue *, const std::string &path = "");
  AJsonValidator *clone() const;
  ~ORValidator();
};

struct funcCheck {
  bool (*func)(const JsonString &);
  IChecker *checker;
  std::string error;
  funcCheck() : func(0), checker(0) {}
};

class StringValidator : public AJsonValidator {
private:
  size_t min_;
  size_t max_;
  bool checkMin;
  bool checkMax;
  std::map<std::string, funcCheck> checkers;
  std::string defaultValue_;

public:
  StringValidator();
  StringValidator(const StringValidator &);
  AJsonValidator *clone() const;
  StringValidator &min(size_t);
  StringValidator &max(size_t);
  StringValidator &isIpv4();
  StringValidator &isPort();
  StringValidator &isValidPath();
  StringValidator &isStartWith(char);
  StringValidator &isEqual(const std::string &);
  StringValidator &isFileWithPermissions(int);
  StringValidator &isDirWithPermissions(int);
  StringValidator &isValidDir();
  StringValidator &isValidFile();
  StringValidator &isDigit();
  StringValidator &notEmpty();
  StringValidator &optional();
  bool validate(const AJsonValue *, const std::string &path = "");
  AJsonValue *get_default() const;
  StringValidator &withDefault(const std::string);
  ~StringValidator();
};

class NumberValidator : public AJsonValidator {
private:
  long min_;
  long max_;
  bool checkMin;
  bool checkMax;
  long defaultValue_;

public:
  NumberValidator();
  NumberValidator(const NumberValidator &);
  AJsonValidator *clone() const;
  NumberValidator &min(long);
  NumberValidator &max(long);
  NumberValidator &range(long, long);
  NumberValidator &range(long);
  NumberValidator &optional();
  bool validate(const AJsonValue *, const std::string &path = "");
  AJsonValue *get_default() const;
  NumberValidator &withDefault(long);
  ~NumberValidator();
};

class BoolValidator : public AJsonValidator {

public:
  BoolValidator();
  BoolValidator(const BoolValidator &);
  BoolValidator &optional();
  bool validate(const AJsonValue *, const std::string &path = "");
  AJsonValue *get_default() const;
  AJsonValidator *clone() const;
  BoolValidator &withDefault(bool);
  ~BoolValidator();
};

ObjectValidator obj();

StringValidator str();

NumberValidator num();

ArrayValidator arr();

ORValidator Or();

BoolValidator Bool();
