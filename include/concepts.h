#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace json {

    class json_value;

    using json_null_t = std::nullptr_t;
    using json_bool_t = bool;
    using json_int_t = int;
    using json_double_t = double;
    using json_string_t = std::string;

    using json_array = std::vector<json_value>;
    using json_object = std::unordered_map<json_string_t, json_value>;    

    namespace concepts {

        template <typename T>
        concept is_json_value = 
            std::is_same_v<T, json_null_t> ||
            std::is_same_v<T, json_bool_t> ||
            std::is_same_v<T, json_int_t> ||
            std::is_same_v<T, json_double_t> ||
            std::is_same_v<T, json_string_t> ||
            std::is_same_v<T, json_array> ||
            std::is_same_v<T, json_object>;

    } // namespace concepts

} // namespace json