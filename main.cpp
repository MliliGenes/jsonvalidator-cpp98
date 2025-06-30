#include "Json.hpp"
#include "JsonValidator.hpp"
#include "schema.cpp"
#include <iostream>

int main() {
  AJsonValue *config = Json::parse("config.json");

  if (!ServerSchema.validate(config)) {
    const std::vector<ValidationError> &errors = ServerSchema.getErrors();
    for (size_t i = 0; i < errors.size(); ++i) {
      std::cout << errors[i].path << ": " << errors[i].msg << std::endl;
    }
  } else {
    std::cout << "JSON config is valid!" << std::endl;
  }
  delete config;
}
