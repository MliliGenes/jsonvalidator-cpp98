#pragma once

#include "AJsonValue.hpp"

class Json {
public:
  static AJsonValue *parse(std::string);
  static AJsonValue *parse_raw(std::string);
};

std::string match_json_name(json_type type);
