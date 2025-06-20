#pragma once


#include <variant>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

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

    class json_value {
    public:

        using value_type = std::variant<
            json_null_t,
            json_bool_t,
            json_int_t,
            json_double_t,
            json_string_t,
            json_array,
            json_object
        >;

    public:

        json_value() :
            value_{ nullptr }
        {
        }

        template <typename T>
            requires (concepts::is_json_value<T>)
        json_value(T value) :
            value_{ std::move(value) }
        {
        }

        json_value(const char* value) :
            json_value(std::string(value))
        {
        }

        json_value(std::initializer_list<json_value> list)
        {
            json_array tmp;
            tmp.reserve(list.size());
            for (const auto& item : list) {
                tmp.push_back(item);
            }
            value_ = std::move(tmp);
        }

        json_value(const json_value&) = default;
        json_value(json_value&&) noexcept = default;
        json_value& operator=(const json_value&) = default;
        json_value& operator=(json_value&&) noexcept = default;

    public:

        template <typename T>
            requires concepts::is_json_value<T>
        T& as() {
            return std::get<T>(value_);
        }

        template <typename T>
            requires concepts::is_json_value<T>
        const T& as() const {
            return const_cast<json_value*>(this)->as<T>();
        }

        template <typename T>
            requires concepts::is_json_value<T>
        T* try_as() {
            return is<T>() ? &std::get<T>(value_) : nullptr;
        }

        template <typename T>
            requires concepts::is_json_value<T>
        const T* try_as() const {
            return const_cast<json_value*>(this)->try_as<T>();
        }

        template <typename T>
            requires concepts::is_json_value<T>
        bool is() const {
            return std::holds_alternative<T>(value_);
        }

    private:

        value_type value_;

    };

    class json_document {
    public:

        json_document() = default;
        json_document(const json_document&) = default;
        json_document(json_document&&) noexcept = default;
        json_document& operator=(const json_document&) = default;
        json_document& operator=(json_document&&) noexcept = default;

    public:

        template <typename T>
            requires concepts::is_json_value<T>
        explicit json_document(T root) :
            root_{ std::move(root) }
        {
        }

        json_document(std::initializer_list<json_value> list) {
            json_array tmp;
            tmp.reserve(list.size());
            for (const auto& item : list) {
                tmp.push_back(item);
            }
            root_ = std::move(tmp);
        }

    public:

        json_value& root() {
            return root_;
        }

        const json_value& root() const {
            return const_cast<json_document*>(this)->root();
        }

        bool empty() const {
            return root_.is<json_null_t>() || (root_.is<json_array>() && root_.as<json_array>().empty()) ||
                (root_.is<json_object>() && root_.as<json_object>().empty());
        }

        void set(json_value root) {
            root_ = std::move(root);
        }

        void clear() {
            root_ = json_value{};
        }

    public:

        std::string to_string() const {
            return format_value_(root_);
        }

        void from_string(const std::string& str) {

        }

    private:

        std::string format_string_(const json_string_t& value) const {
            std::string result;
            result.reserve(value.size() + 2);
            result += '"';
            for (char c : value) {
                switch (c) {
                    case '\"': result += "\\\""; break;
                    case '\\': result += "\\\\"; break;
                    case '\b': result += "\\b"; break;
                    case '\f': result += "\\f"; break;
                    case '\n': result += "\\n"; break;
                    case '\r': result += "\\r"; break;
                    case '\t': result += "\\t"; break;
                    default:
                        if (static_cast<unsigned char>(c) < 0x20) {
                            char buf[7];
                            snprintf(buf, sizeof(buf), "\\u%04x", c & 0xFF);
                            result += buf;
                        }
                        else {
                            result += c;
                        }
                }
            }
            result += '"';
            return result;
        }

        std::string format_array_(const json_array& value, int indent_level = 0, int indent_step = 4) const {
            if (value.empty()) {
                return "[ ]";
            }

            std::string indent(indent_level * indent_step, ' ');
            std::string next_indent((indent_level + 1) * indent_step, ' ');
            std::ostringstream result; result << "[\n";

            for (std::size_t i{ 0 }; i < value.size(); ++i) {
                result << next_indent << format_value_(value[i], indent_level + 1, indent_step);
                if (i + 1 < value.size()) {
                    result << ",";
                }
                result << "\n";
            }
            result << indent << "]";
            return result.str();
        }

        std::string format_object_(const json_object& value, int indent_level = 0, int indent_step = 4) const {
            if (value.empty()) {
                return "{ }";
            }
            std::string indent(indent_level * indent_step, ' ');
            std::string next_indent((indent_level + 1) * indent_step, ' ');
            std::ostringstream result;
            result << "{\n";
            std::size_t count{ 0 };
            for (const auto& [k, v] : value) {
                result << next_indent << format_string_(k) << ": " << format_value_(v, indent_level + 1, indent_step);
                if (++count < value.size()) result << ",";
                result << "\n";
            }
            result << indent << "}";

            return result.str();
        }

        std::string format_value_(const json_value& value, int indent_level = 0, int indent_step = 4) const {
            std::string indent(indent_level * indent_step, ' ');
            std::string next_indent((indent_level + 1) * indent_step, ' ');
            if (value.is<json_null_t>()) {
                return "null";
            }
            else if (value.is<json_bool_t>()) {
                return value.as<json_bool_t>() ? "true" : "false";
            }
            else if (value.is<json_int_t>()) {
                return std::to_string(value.as<json_int_t>());
            }
            else if (value.is<json_double_t>()) {
                std::ostringstream oss;
                oss.precision(15);
                oss << value.as<json_double_t>();
                return oss.str();
            }
            else if (value.is<json_string_t>()) {
                return format_string_(value.as<json_string_t>());
            }
            else if (value.is<json_array>()) {
                return format_array_(value.as<json_array>(), indent_level, indent_step);
            }
            else if (value.is<json_object>()) {
                return format_object_(value.as<json_object>(), indent_level, indent_step);
            }
            return {};
        }

    private:

        json_value root_{};

    };


} // namespace json