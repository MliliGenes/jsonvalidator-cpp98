# C++98 JSON Parser & Validator (Zod-like)
This project is a custom JSON parser and schema validator library written in C++98, designed without relying on modern C++ features or any external JSON libraries.

Due to the lack of built-in JSON support and modern language features in C++98, the project includes:

* A full JSON parser
* A custom JSON object model (JsonObject, JsonArray, etc.)
* A Zod-like schema validator with fluent validation rules

## Why? 🤔
This project originally started as a fun side project, where I wanted to challenge myself to build a JSON parser and validator from scratch in C++98.

Later, I decided to integrate it into my 1337 (42) school `webserv` project to handle server configuration files using JSON instead of the more common `.conf` or `YAML` formats that most students choose.

Since the webserv project at 42 requires strict C++98 compliance, I couldn’t rely on any modern JSON libraries. This pushed me to design and implement my own:
- JSON Object Model
- JSON Parser
- JSON Schema Validator (inspired by tools like Zod in TypeScript)

Rather than taking the common path of writing an NGINX-style parser, I wanted something developer-friendly and schema-validated, making configuration parsing both fun and robust.

## Features ✅
- JSON parsing from file or raw string
- Full JSON type system: Object, Array, String, Number, Bool, Null
- Chainable & type-safe JSON validation
- Schema definitions for complex nested JSON structures
- Default values support
- Optional fields
- Custom error reporting (Not fully supported 🗿)
- Supports constraints like min, max, string length, allowed values, etc.
- Object shape checking, array item validation, OR conditions, and more

## Example: Server Configuration Schema ✅
#### Parsing JSON
```cpp
AJsonValue* json = Json::parse("config.json");  // From file
AJsonValue* jsonRaw = Json::parse_raw("{ \"key\": \"value\" }");  // From string
```

#### Schema definition (like Zod in JS):
```cpp
ArrayValidator ServerSchema = arr().item(
    obj()
        .property("server_name", str().notEmpty())
        .property("host", str().notEmpty())
        .property("port", arr().item(num().min(0).max(65535)).min(1))
        .property("redirects", obj().match(str().isStartWith('/'),str().isStartWith('/'))).optional()
        .property("error_pages", obj().match(str().isDigit(), str().notEmpty())).optional()
        .property("default_root", str().notEmpty().isValidDir())
        .property("default_index", arr().item(str().notEmpty()))
        .property("location", obj().match(str().notEmpty().isStartWith('/'), locationSchema))
        .property("list_dir", Bool().withDefault(false))
        .property("client_max_body_size", num().min(0).withDefault(1024)))
    .min(1);
```
#### Location schema example (nested object validation):

```cpp
ObjectValidator locationSchema = obj()
    .property("allowed_methods", arr().item(Or()
        .addConditions(str().isEqual("GET"))
        .addConditions(str().isEqual("POST"))
        .addConditions(str().isEqual("DELETE"))
        .withMsg("Allowed methods must be GET, POST or DELETE!")).max(3))
    .property("auto_index", Bool().withDefault(false))
    .property("cgi", obj().match(str().notEmpty(), str().notEmpty().isValidPath()))
    .property("upload_dir", str().notEmpty()).optional();
```
### Example Usage ✅

```cpp
#include "Json.hpp"
#include "JsonValidator.hpp"

AJsonValue* config = Json::parse("config.json");

if (!ServerSchema.validate(config)) {
    const std::vector<ValidationError>& errors = ServerSchema.getErrors();
    for (size_t i = 0; i < errors.size(); ++i) {
        std::cout << errors[i].path << "': " << errors[i].msg << std::endl;
    }
} else {
    std::cout << "JSON config is valid!" << std::endl;
}

delete config;
```
## Limitations ❌
- C++98: No modern C++ features like smart pointers, STL JSON libs, or variadic templates
- Manual Memory Management: You are responsible for deleting JSON values after use
- No Exceptions: Errors are handled via error reporting, not exceptions
- No Streaming / Incremental Parsing: Full JSON loads into memory

## Building 🔨

Example (G++ or Clang with C++98 mode):
```bash
c++ -std=c++98 -Iincludes src/*.cpp -o json_validator main.cpp
```
Where main.cpp is your file that uses the library.

## License 📜
This project is licensed under the MIT License ©️ Mohammed Hayyoun.
