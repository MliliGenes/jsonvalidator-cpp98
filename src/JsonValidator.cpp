#include "JsonValidator.hpp"
#include "AJsonValue.hpp"
#include "JsonTypes.hpp"
#include "utils.hpp"
#include "validators.hpp"

ValidationError::ValidationError(const std::string &p, const std::string &m)
    : path(p), msg(m) {}

AJsonValidator::AJsonValidator()
    : optional_(false), hasDefault_(false), exceptedType_(UNDEFINED),
      defaultValue_(NULL) {}

AJsonValidator::AJsonValidator(const AJsonValidator &obj)
    : optional_(obj.optional_), hasDefault_(obj.hasDefault_),
      exceptedType_(obj.exceptedType_), defaultValue_(obj.defaultValue_) {}

AJsonValue *AJsonValidator::get_default() const {
  return defaultValue_ ? defaultValue_->clone() : NULL;
}

bool AJsonValidator::has_default() const { return hasDefault_; }

void AJsonValidator::set_default(AJsonValue *value) {
  if (defaultValue_)
    delete defaultValue_;
  defaultValue_ = value ? value->clone() : NULL;
  hasDefault_ = true;
}

AJsonValue *AJsonValidator::applyDefaults(AJsonValue *v) {
  if (v && !v->isNull()) {
    return v;
  }
  if (hasDefault_) {
    AJsonValue *defaultVal = get_default();
    if (v) {
      *v = *defaultVal;
      delete defaultVal;
      return v;
    }
    return defaultVal;
  }
  return v;
}

void AJsonValidator::set_optional() { optional_ = true; }

bool AJsonValidator::get_optional() const { return optional_; }

bool AJsonValidator::isTypeCompatible(const AJsonValidator &v,
                                      const AJsonValue *j) const {
  return j->getType() == v.exceptedType_;
}

ObjectValidator &ObjectValidator::optional() {
  if (last_)
    last_->set_optional();
  return (*this);
}

StringValidator &StringValidator::optional() {
  optional_ = true;
  return (*this);
}

const std::vector<ValidationError> &AJsonValidator::getErrors() const {
  return errors_;
}

void AJsonValidator::clearErrors() { errors_.clear(); }
AJsonValidator::~AJsonValidator() {}

void AJsonValidator::addError(const std::string &path, const std::string &msg) {
  errors_.push_back(ValidationError(path, msg));
}

TypeValidator::TypeValidator(json_type exceptedType) {
  exceptedType_ = exceptedType;
}

bool TypeValidator::validate(const AJsonValue *v, const std::string &path) {
  if (v->getType() == exceptedType_)
    return true;
  std::string msg;
  msg = "Excepted: " + match_json_name(exceptedType_);
  msg += ", got: " + match_json_name(v->getType());
  addError(path, msg);
  return false;
}

AJsonValidator *TypeValidator::clone() const {
  return new TypeValidator(*this);
}

ObjectValidator::ObjectValidator()
    : allowAdditional_(true), matchMode_(false), emptyCheck(false),
      key_validator(NULL), val_validator(NULL), last_(NULL) {
  exceptedType_ = OBJECT;
}

ObjectValidator::ObjectValidator(const ObjectValidator &obj)
    : allowAdditional_(obj.allowAdditional_), matchMode_(obj.matchMode_),
      emptyCheck(obj.emptyCheck), key_validator(NULL), val_validator(NULL),
      last_(NULL) {
  if (obj.val_validator)
    val_validator = obj.val_validator->clone();
  if (obj.key_validator)
    key_validator = dynamic_cast<StringValidator *>(obj.key_validator->clone());
  optional_ = obj.optional_;
  exceptedType_ = OBJECT;
  hasDefault_ = obj.hasDefault_;
  if (obj.defaultValue_)
    defaultValue_ = dynamic_cast<JsonObject *>(obj.defaultValue_->clone());
  ValidatorMap::const_iterator it = obj.properties_.begin();
  for (; it != obj.properties_.end(); it++) {
    properties_[it->first] = it->second->clone();
    if (obj.last_ == it->second)
      last_ = properties_[it->first];
  }
}

AJsonValidator *ObjectValidator::clone() const {
  return new ObjectValidator(*this);
}

ObjectValidator &ObjectValidator::property(const std::string &name,
                                           const AJsonValidator &v) {
  if (matchMode_)
    return *this;
  if (properties_[name]) {
    delete properties_[name];
  }
  properties_[name] = v.clone();
  last_ = properties_[name];
  return *this;
}

ObjectValidator &ObjectValidator::allowAdditional(bool allow) {
  allowAdditional_ = allow;
  return *this;
}

ObjectValidator &ObjectValidator::match(const StringValidator &sv,
                                        const AJsonValidator &v) {
  matchMode_ = true;
  key_validator = dynamic_cast<StringValidator *>(sv.clone());
  val_validator = v.clone();
  return *this;
}

ObjectValidator &ObjectValidator::notEmpty() {
  emptyCheck = true;
  return *this;
}

bool ObjectValidator::validate(const AJsonValue *v, const std::string &path) {
  TypeValidator typeCheck(OBJECT);
  if (!typeCheck.validate(v, path)) {
    errors_ = typeCheck.getErrors();
    return false;
  }
  applyDefaults(const_cast<AJsonValue *>(v));
  JsonObject *obj = (v)->asObject();
  bool valid = true;
  if (emptyCheck && obj->size() < 1) {
    addError(path, "Object must not be empty!");
    return false;
  }
  if (matchMode_) {
    JsonObject::const_iterator it = obj->begin();
    for (; it != obj->end(); it++) {
      const JsonString string = JsonString(it->first);
      std::string propPath =
          (path.empty() ? it->first : path + ".<" + it->first) + '>';
      if (!key_validator->validate(&string, propPath)) {
        AJsonValidator::ValidationErr keyErr = key_validator->getErrors();
        errors_.insert(errors_.end(), keyErr.begin(), keyErr.end());
        key_validator->clearErrors();
        valid = false;
      }
      if (!val_validator->validate(it->second, propPath)) {
        AJsonValidator::ValidationErr valErr = val_validator->getErrors();
        errors_.insert(errors_.end(), valErr.begin(), valErr.end());
        val_validator->clearErrors();
        valid = false;
      }
    }
    return valid;
  }
  ValidatorMap::const_iterator it = properties_.begin();
  for (; it != properties_.end(); it++) {
    std::string propPath = (path.empty() ? it->first : path + "." + it->first);
    AJsonValue &propValue = (*v)[it->first];
    if (!propValue) {
      if (it->second->get_optional())
        continue;
      addError(propPath, "Missing Field!");
      valid = false;
      continue;
    }
    if (!it->second->validate(&propValue, propPath)) {
      AJsonValidator::ValidationErr propsErrs = it->second->getErrors();
      errors_.insert(errors_.end(), propsErrs.begin(), propsErrs.end());
      it->second->clearErrors();
      valid = false;
    }
  }
  if (!allowAdditional_) {
    JsonObject::const_iterator o_it = obj->begin();
    for (; o_it != obj->end(); o_it++) {
      if (properties_.find(o_it->first) == properties_.end()) {
        std::string propPath =
            (path.empty() ? o_it->first : path + "." + o_it->first);
        addError(propPath, "Unexpected property");
        valid = false;
      }
    }
  }
  return valid;
}

ObjectValidator &ObjectValidator::withDefault(JsonObject &v) {
  set_default(&v);
  return *this;
}

AJsonValue *ObjectValidator::applyDefaults(AJsonValue *v) {
  AJsonValue *result = AJsonValidator::applyDefaults(v);
  if (result != v) {
    return result;
  }

  if (!v || v->getType() != OBJECT) {
    return v;
  }

  JsonObject *obj = v->asObject();
  for (ValidatorMap::iterator it = properties_.begin(); it != properties_.end();
       ++it) {
    AJsonValue &propValue = (*obj)[it->first];
    if (!propValue && it->second->has_default()) {
      obj->setProperty(it->first, it->second->get_default());
    } else if (!propValue.isEmpty()) {
      AJsonValue *newValue = it->second->applyDefaults(&propValue);
      if (newValue != &propValue) {
        obj->replaceProperty(it->first, newValue);
      }
    }
  }
  return v;
}

ObjectValidator::~ObjectValidator() {
  ValidatorMap::iterator it = properties_.begin();
  for (; it != properties_.end(); it++)
    delete it->second;
  if (key_validator)
    delete key_validator;
  if (val_validator)
    delete val_validator;
  if (defaultValue_)
    delete defaultValue_;
}

ArrayValidator::ArrayValidator()
    : validator_(NULL), min_(0), max_(SIZE_T_MAX), defaultValue_(NULL) {
  exceptedType_ = ARRAY;
}

ArrayValidator::ArrayValidator(const ArrayValidator &obj)
    : validator_(NULL), min_(obj.min_), max_(obj.max_), defaultValue_(NULL) {
  exceptedType_ = ARRAY;
  hasDefault_ = obj.hasDefault_;
  if (obj.defaultValue_)
    defaultValue_ = dynamic_cast<JsonArray *>(obj.defaultValue_->clone());
  if (obj.validator_) {
    validator_ = obj.validator_->clone();
  }
}

ArrayValidator &ArrayValidator::item(const AJsonValidator &v) {
  if (validator_)
    delete validator_;
  validator_ = v.clone();
  return *this;
}

ArrayValidator &ArrayValidator::min(size_t size) {
  min_ = size;
  return *this;
}

ArrayValidator &ArrayValidator::max(size_t size) {
  max_ = size;
  return *this;
}

ArrayValidator &ArrayValidator::optional() { return *this; }

bool ArrayValidator::validate(const AJsonValue *v, const std::string &path) {
  TypeValidator checkType(ARRAY);
  if (!checkType.validate(v, path)) {
    errors_ = checkType.getErrors();
    return false;
  }
  applyDefaults(const_cast<AJsonValue *>(v));
  JsonArray &array = *v->asArray();
  bool valid = true;

  if (array.size() < min_) {
    addError("[]" + path, "Array too small (min " + to_string(min_) + ")");
    valid = false;
  }

  if (array.size() > max_) {
    addError("[]" + path, "Array too large (max " + to_string(max_) + ")");
    valid = false;
  }
  if (validator_) {
    for (unsigned long i = 0; i < array.size(); i++) {
      std::string itemPath = path + "[" + to_string(i) + "]";
      if (!validator_->validate(&array[i], itemPath)) {
        const ValidationErr &tmp_errors = validator_->getErrors();
        errors_.insert(errors_.end(), tmp_errors.begin(), tmp_errors.end());
        validator_->clearErrors();
        valid = false;
      }
    }
  }
  return valid;
}

AJsonValue *ArrayValidator::get_default() const {
  return defaultValue_ ? defaultValue_->clone() : new JsonArray();
}

ArrayValidator &ArrayValidator::withDefault(JsonArray &v) {
  hasDefault_ = true;
  defaultValue_ = dynamic_cast<JsonArray *>(v.clone());
  return *this;
}

AJsonValue *ArrayValidator::applyDefaults(AJsonValue *v) {
  AJsonValue *result = AJsonValidator::applyDefaults(v);
  if (result != v) {
    return result;
  }

  if (!v || v->getType() != ARRAY || !validator_) {
    return v;
  }

  JsonArray *arr = v->asArray();
  for (size_t i = 0; i < arr->size(); i++) {
    AJsonValue *newValue = validator_->applyDefaults(&(*arr)[i]);
    if (newValue != &(*arr)[i]) {
      (*arr)[i] = *newValue;
      delete newValue;
    }
  }
  return v;
}

AJsonValidator *ArrayValidator::clone() const {
  return new ArrayValidator(*this);
}

ArrayValidator::~ArrayValidator() {
  if (validator_)
    delete validator_;
  if (defaultValue_)
    delete defaultValue_;
}

ORValidator::ORValidator() {}

ORValidator::ORValidator(const ORValidator &obj) : msg_(obj.msg_) {
  for (unsigned long i = 0; i < obj.conditions_.size(); i++) {
    conditions_.push_back(obj.conditions_[i]->clone());
  }
}

ORValidator &ORValidator::addConditions(const AJsonValidator &v) {
  conditions_.push_back(v.clone());
  return *this;
}

bool ORValidator::validate(const AJsonValue *v, const std::string &path) {
  if (conditions_.empty()) {
    addError(path, "No conditions specified in ANY validator");
    return false;
  }

  ValidationErr betterErr;
  bool anyValid = false;

  for (unsigned long i = 0; i < conditions_.size(); i++) {
    AJsonValidator *validator = conditions_[i];
    if (validator->validate(v, path))
      return true;
    const ValidationErr &errs = validator->getErrors();
    if (!errs.empty() && isTypeCompatible(*validator, v)) {
      if (!anyValid || errs.size() < betterErr.size()) {
        betterErr = errs;
        anyValid = true;
      }
    } else if (!anyValid && betterErr.empty()) {
      betterErr = errs;
    }
    validator->clearErrors();
  }

  if (!anyValid) {
    addError(path, "Value didn't match any ANY conditions");
  }

  if (msg_.empty()) {
    errors_ = betterErr;
  } else {
    errors_.clear();
    errors_.push_back(ValidationError(path, msg_));
  }

  return false;
}

ORValidator &ORValidator::withMsg(const std::string &msg) {
  msg_ = msg;
  return *this;
}

AJsonValidator *ORValidator::clone() const { return new ORValidator(*this); }

ORValidator::~ORValidator() {
  for (unsigned long i = 0; i < conditions_.size(); i++) {
    delete conditions_[i];
  }
}

StringValidator::StringValidator()
    : min_(0), max_(0), checkMin(false), checkMax(false) {
  exceptedType_ = STRING;
}

StringValidator::StringValidator(const StringValidator &obj)
    : min_(obj.min_), max_(obj.max_), checkMin(obj.checkMin),
      checkMax(obj.checkMax), defaultValue_(obj.defaultValue_) {
  optional_ = obj.optional_;
  exceptedType_ = STRING;
  hasDefault_ = obj.hasDefault_;
  std::map<std::string, funcCheck>::const_iterator it = obj.checkers.begin();
  for (; it != obj.checkers.end(); it++) {
    funcCheck copied = it->second;
    if (copied.checker)
      copied.checker = copied.checker->clone();
    checkers[it->first] = copied;
  }
}

AJsonValidator *StringValidator::clone() const {
  return new StringValidator(*this);
}

StringValidator &StringValidator::min(size_t size) {
  checkMin = true;
  min_ = size;
  return *this;
}

StringValidator &StringValidator::isDigit() {
  funcCheck check;
  check.func = isDigit_;
  check.error = "Must contain only digits!";
  checkers["isDigit"] = check;
  return *this;
}

StringValidator &StringValidator::isIpv4() {
  funcCheck check;
  check.func = isIpv4_;
  check.error = "Must be a valid IP address!";
  checkers["isIpv4"] = check;
  return *this;
}

StringValidator &StringValidator::isPort() {
  funcCheck check;
  check.func = isPort_;
  check.error = "Must be a valid port!";
  checkers["isPort"] = check;
  return *this;
}

StringValidator &StringValidator::isValidPath() {
  funcCheck check;
  check.func = isValidPath_;
  check.error = "Must be a valid path!";
  checkers["isValidPath"] = check;
  return *this;
}

StringValidator &StringValidator::isStartWith(char c) {
  funcCheck check;
  check.checker = new isStartWith_(c);
  check.error = "Must start with '" + std::string(1, c) + "'!";
  checkers["isStartWith"] = check;
  return *this;
}

StringValidator &StringValidator::isEqual(const std::string &value) {
  funcCheck check;
  check.checker = new isEqual_(value);
  check.error = "Must be equal to '" + value + "'!";
  checkers["isEqual"] = check;
  return *this;
}

StringValidator &StringValidator::isFileWithPermissions(int permissions) {
  funcCheck check;
  check.checker = new isFileWithPermissions_(permissions);
  check.error =
      "Must be a file with permissions " + perm_to_string(permissions) + "!";
  checkers["isFileWithPermissions"] = check;
  return *this;
}

StringValidator &StringValidator::isDirWithPermissions(int permissions) {
  funcCheck check;
  check.checker = new isDirWithPermissions_(permissions);
  check.error = "Must be a directory with permissions " +
                perm_to_string(permissions) + "!";
  checkers["isDirWithPermissions"] = check;
  return *this;
}

StringValidator &StringValidator::isValidDir() {
  funcCheck check;
  check.func = isValidDir_;
  check.error = "Must be a valid directory!";
  checkers["isValidDir"] = check;
  return *this;
}

StringValidator &StringValidator::isValidFile() {
  funcCheck check;
  check.func = isValidFile_;
  check.error = "Must be a valid file!";
  checkers["isValidFile"] = check;
  return *this;
}

StringValidator &StringValidator::max(size_t size) {
  checkMax = true;
  max_ = size;
  return *this;
}

StringValidator &StringValidator::notEmpty() {
  funcCheck check;
  check.func = isEmpty_;
  check.error = "Must not be empty!";
  checkers["notEmpty"] = check;
  return *this;
}

bool StringValidator::validate(const AJsonValue *v, const std::string &path) {
  TypeValidator typeCheck(STRING);
  if (!typeCheck.validate(v, path)) {
    errors_ = typeCheck.getErrors();
    return false;
  }
  std::string value = v->asString();
  if (checkMin && value.length() < min_) {
    addError(path,
             "String must have at least (" + to_string(min_) + ") chars!");
    return false;
  }
  if (checkMax && value.length() > max_) {
    addError(path, "String must have at most (" + to_string(max_) + ") chars!");
    return false;
  }
  const JsonString &desiredType = dynamic_cast<const JsonString &>(*v);
  if (!checkers.empty()) {
    std::map<std::string, funcCheck>::iterator it = checkers.begin();
    for (; it != checkers.end(); it++) {
      if (it->second.func) {
        if (!it->second.func(desiredType)) {
          addError(path, it->second.error);
          return false;
        }
      } else if (it->second.checker) {
        if (!(*it->second.checker)(desiredType)) {
          addError(path, it->second.error);
          return false;
        }
      }
    }
  }
  return true;
}

AJsonValue *StringValidator::get_default() const {
  return new JsonString(defaultValue_);
}

StringValidator &StringValidator::withDefault(const std::string value) {
  hasDefault_ = true;
  defaultValue_ = value;
  return *this;
}

StringValidator::~StringValidator() {
  std::map<std::string, funcCheck>::iterator it = checkers.begin();
  for (; it != checkers.end(); it++) {
    if (it->second.checker)
      delete it->second.checker;
  }
  checkers.clear();
}

NumberValidator::NumberValidator()
    : min_(0), max_(0), checkMin(false), checkMax(false), defaultValue_(0) {
  exceptedType_ = NUMBER;
}

NumberValidator::NumberValidator(const NumberValidator &obj)
    : min_(obj.min_), max_(obj.max_), checkMin(obj.checkMin),
      checkMax(obj.checkMax), defaultValue_(obj.defaultValue_) {
  optional_ = obj.optional_;
  exceptedType_ = NUMBER;
  hasDefault_ = obj.hasDefault_;
}

AJsonValidator *NumberValidator::clone() const {
  return new NumberValidator(*this);
}

NumberValidator &NumberValidator::min(long size) {
  min_ = size;
  checkMin = true;
  return *this;
}

NumberValidator &NumberValidator::max(long size) {
  max_ = size;
  checkMax = true;
  return *this;
}

NumberValidator &NumberValidator::range(long n1, long n2) {
  min_ = n1;
  max_ = n2;
  if (n1 > n2) {
    min_ = n2;
    max_ = n1;
  }
  checkMax = true;
  checkMin = true;
  return *this;
}

NumberValidator &NumberValidator::range(long size) {
  min_ = 0;
  max_ = size;
  checkMax = true;
  checkMin = true;
  return *this;
}

NumberValidator &NumberValidator::optional() {
  optional_ = true;
  return *this;
}

bool NumberValidator::validate(const AJsonValue *v, const std::string &path) {
  TypeValidator typeCheck(NUMBER);
  if (!typeCheck.validate(v, path)) {
    errors_ = typeCheck.getErrors();
    return false;
  }
  if (checkMin && checkMax && (v->asNumber() < min_ || v->asNumber() > max_)) {
    addError(path, "Number must be between (" + to_string(min_) + ") and (" +
                       to_string(max_) + ")!");
    return false;
  }
  if (checkMin && v->asNumber() < min_) {
    addError(path,
             "Number must be big than or equal to (" + to_string(min_) + ")!");
    return false;
  }
  if (checkMax && v->asNumber() > max_) {
    addError(path,
             "Number must be less than or equal to (" + to_string(max_) + ")!");
    return false;
  }
  return true;
}

AJsonValue *NumberValidator::get_default() const {
  return new JsonNumber(to_string(defaultValue_));
}

NumberValidator &NumberValidator::withDefault(long val) {
  hasDefault_ = true;
  defaultValue_ = val;
  return *this;
}

NumberValidator::~NumberValidator() {}

BoolValidator::BoolValidator() {}

BoolValidator::BoolValidator(const BoolValidator &obj) {
  hasDefault_ = obj.hasDefault_;
  defaultValue_ = obj.get_default();
}

BoolValidator &BoolValidator::optional() {
  optional_ = true;
  return *this;
}

bool BoolValidator::validate(const AJsonValue *v, const std::string &path) {
  TypeValidator typeCheck(BOOLEAN);
  if (!typeCheck.validate(v, path)) {
    errors_ = typeCheck.getErrors();
    return false;
  }
  return true;
}

AJsonValidator *BoolValidator::clone() const {
  return new BoolValidator(*this);
}

AJsonValue *BoolValidator::get_default() const {
  return hasDefault_ ? new JsonBool((*defaultValue_).asBool()) : NULL;
}

BoolValidator &BoolValidator::withDefault(bool value) {
  JsonBool b(value);
  set_default(&b);
  return *this;
}

BoolValidator::~BoolValidator() {
  if (defaultValue_)
    delete defaultValue_;
}

ObjectValidator obj() { return ObjectValidator(); }
StringValidator str() { return StringValidator(); }
NumberValidator num() { return NumberValidator(); }
ArrayValidator arr() { return ArrayValidator(); }
ORValidator Or() { return ORValidator(); }
BoolValidator Bool() { return BoolValidator(); }
