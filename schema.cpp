#include "schema.hpp"

ObjectValidator locationSchema = obj()
    .property("allowed_methods",arr().item(
        Or()
        .addConditions(str().isEqual("GET"))
        .addConditions(str().isEqual("POST"))
        .addConditions(str().isEqual("DELETE"))
        .withMsg("Allowed methods must be GET, POST or DELETE!"))
        .max(3))
    .property("auto_index", Bool().withDefault(false))
    .property("cgi", obj().match(str().notEmpty(), str().notEmpty().isValidPath())
    .property("upload_dir", str().notEmpty())).optional();

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
