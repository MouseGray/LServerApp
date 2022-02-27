#include "jsonutils.h"
//http://5.100.98.127/ftp/%D0%A2%D0%B5%D1%85%D0%BD%D0%BE%D0%BB%D0%BE%D0%B3%D0%B8%D1%8F%20%D0%BF%D1%80%D0%BE%D0%B3%D1%80%D0%B0%D0%BC%D0%BC%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D1%8F/BOOKS/%D0%A7%D0%B0%D1%80%D0%BB%D1%8C%D0%B7%20%D0%9F%D0%B5%D1%82%D1%86%D0%BE%D0%BB%D1%8C%D0%B4%20-%20%D0%9A%D0%BE%D0%B4_%20%D1%82%D0%B0%D0%B9%D0%BD%D1%8B%D0%B9%20%D1%8F%D0%B7%D1%8B%D0%BA%20%D0%B8%D0%BD%D1%84%D0%BE%D1%80%D0%BC%D0%B0%D1%82%D0%B8%D0%BA%D0%B8-%D0%9C%D0%B0%D0%BD%D0%BD,%20%D0%98%D0%B2%D0%B0%D0%BD%D0%BE%D0%B2%20%D0%B8%20%D0%A4%D0%B5%D1%80%D0%B1%D0%B5%D1%80%20(2019).pdf
template <>
const char* jsonutils::type_text<jsonutils::Type::Null>() {
    return "Null";
}

template <>
const char* jsonutils::type_text<jsonutils::Type::Bool>() {
    return "Bool";
}

template <>
const char* jsonutils::type_text<jsonutils::Type::Number>() {
    return "Number";
}

template <>
const char* jsonutils::type_text<jsonutils::Type::String>() {
    return "String";
}

template <>
const char* jsonutils::type_text<jsonutils::Type::Array>() {
    return "Array";
}

template <>
const char* jsonutils::type_text<jsonutils::Type::Object>() {
    return "Object";
}

template <>
bool jsonutils::test_type<jsonutils::Type::Null>(const rapidjson::GenericValue<rapidjson::UTF8<>>& json) {
    return json.IsNull();
}

template <>
bool jsonutils::test_type<jsonutils::Type::Bool>(const rapidjson::GenericValue<rapidjson::UTF8<>>& json) {
    return json.IsBool();
}

template <>
bool jsonutils::test_type<jsonutils::Type::Number>(const rapidjson::GenericValue<rapidjson::UTF8<>>& json) {
    return json.IsNumber();
}

template <>
bool jsonutils::test_type<jsonutils::Type::String>(const rapidjson::GenericValue<rapidjson::UTF8<>>& json) {
    return json.IsString();
}

template <>
bool jsonutils::test_type<jsonutils::Type::Array>(const rapidjson::GenericValue<rapidjson::UTF8<>>& json) {
    return json.IsArray();
}

template <>
bool jsonutils::test_type<jsonutils::Type::Object>(const rapidjson::GenericValue<rapidjson::UTF8<>>& json) {
    return json.IsObject();
}
