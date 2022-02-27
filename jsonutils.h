#pragma once

#include <array>

#include <rapidjson/document.h>

namespace jsonutils {

enum class Type
{
    Null,
    Bool,
    Number,
    String,
    Array,
    Object
};

template < Type Ty >
const char* type_text();

template< Type Ty >
struct JsonNameType {
    const char* name;
};

struct JsonNameTypeResult {
    const char* name = nullptr;
    const char* type = nullptr;
    bool is_null() { return name == nullptr && type == nullptr; }
};

template <Type Ty>
bool test_type(const rapidjson::GenericValue<rapidjson::UTF8<>>& json);

template < std::size_t It, Type Ty >
bool has_field(const rapidjson::GenericValue<rapidjson::UTF8<>>& json,
              const JsonNameType<Ty>& arg, JsonNameTypeResult& error) {
    if (!json.HasMember(arg.name) || !test_type<Ty>(json[arg.name])) {
        error = { arg.name, type_text<Ty>() };
        return false;
    }
    return true;
}

template < typename Args, std::size_t ...It >
bool has_fields_impl(const rapidjson::GenericValue<rapidjson::UTF8<>>& json,
                    const Args& args, JsonNameTypeResult& error, std::index_sequence<It...>) {
    return (has_field<It>(json, std::get<It>(args), error) && ...);
}

template < Type ...Args >
JsonNameTypeResult has_fields(const rapidjson::GenericValue<rapidjson::UTF8<>>& json,
               const std::tuple<JsonNameType<Args>...>& args) {
    JsonNameTypeResult error;
    has_fields_impl(json, args, error, std::make_index_sequence<sizeof... (Args)>{});\
    return error;
}



};
