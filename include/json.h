#pragma once

#include <cstdint>
#include <concepts>
#include <cmath>
#include <variant>
#include <string>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <ranges>
#include <sstream>
#include <memory>
#include <stack>

namespace json {

    class json_value;

    using json_null_t = std::nullptr_t;
    using json_bool_t = bool;
    using json_int_t = long long;
    using json_double_t = double;
    using json_string_t = std::string;

    using json_array = std::vector<json_value>;
    using json_object = std::unordered_map<json_string_t, json_value>;

    namespace concepts {

        template <typename T>
        concept is_json_value =
            std::is_same_v<T, json_null_t> ||
            std::is_same_v<T, json_bool_t> ||
            std::is_integral_v<T> ||
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
            value_(std::move(value))
        {
        }

        json_value(const char* value) :
            json_value(std::string(value))
        {
        }

        explicit json_value(std::initializer_list<json_value> list)
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

    }; // class json_value

    enum class json_token_type : std::uint8_t {
        invalid,
        left_bracket, right_bracket,
        left_brace, right_brace,
        colon, comma,
        string_value, int_value, double_value, bool_value, null_value,
        end_of_file
    };

    constexpr bool is_value_token(json_token_type type) {
        return type == json_token_type::string_value ||
            type == json_token_type::int_value ||
            type == json_token_type::double_value ||
            type == json_token_type::bool_value ||
            type == json_token_type::null_value;
    }

    struct json_token {
        json_token_type type;
        json_value value{};
        bool is_valid{ true };
        std::string error_str{};
    };

    class json_lexer {
    public:

        json_lexer() = delete;
        json_lexer(const json_lexer&) = delete;
        json_lexer(json_lexer&&) = delete;
        json_lexer& operator=(const json_lexer&) = delete;
        json_lexer& operator=(json_lexer&&) = delete;

    public:

        explicit json_lexer(const std::string& src_str) :
            pos_{ 0 },
            source_{ src_str }
        {

        }

        json_token next_token() {
            skip_whitespaces_();
            if (pos_ >= source_.size()) {
                return { json_token_type::end_of_file,"" };
            }

            std::string::value_type ch{ peek_() };

            switch (ch) {
                case '{':
                    advance_();
                    return { json_token_type::left_brace };
                    break;
                case '}':
                    advance_();
                    return { json_token_type::right_brace };
                    break;
                case '[':
                    advance_();
                    return { json_token_type::left_bracket };
                    break;
                case ']':
                    advance_();
                    return { json_token_type::right_bracket };
                    break;
                case ':':
                    advance_();
                    return { json_token_type::colon };
                    break;
                case ',':
                    advance_();
                    return { json_token_type::comma };
                    break;
                case '"':
                    return parse_string_();
                    break;
            }

            if (std::isdigit(ch) || ch == '-') {
                return parse_number_();
            }

            static std::string true_str{ "true" };
            static std::string false_str{ "false" };
            static std::string null_str{ "null" };

            if (starts_with_(true_str)) {
                advance_(true_str.size());
                return { json_token_type::bool_value,json_value(true) };
            }
            if (starts_with_(false_str)) {
                advance_(false_str.size());
                return { json_token_type::bool_value,json_value(false) };
            }
            if (starts_with_(null_str)) {
                advance_(null_str.size());
                return { json_token_type::null_value, json_value(nullptr) };
            }
            return { json_token_type::invalid,json_value(nullptr),
                     false, "Unexpected character: " + std::string(1, ch) };
        }

    private:

        json_token parse_number_() {
            std::string result;
            bool is_float = false;

            if (peek_() == '-') {
                result += '-';
                advance_();
            }

            if (pos_ >= source_.size() || !std::isdigit(peek_())) {
                return { json_token_type::invalid, json_value(nullptr), false, "Invalid number format" };
            }

            if (peek_() == '0') {
                result += '0';
                advance_();
                if (pos_ < source_.size() && std::isdigit(peek_())) {
                    return { json_token_type::invalid, json_value(nullptr), false, "Leading zeros are not allowed" };
                }
            }
            else {
                while (pos_ < source_.size() && std::isdigit(peek_())) {
                    result += peek_();
                    advance_();
                }
            }

            if (peek_() == '.') {
                is_float = true;
                result += '.';
                advance_();
                if (pos_ >= source_.size() || !std::isdigit(peek_())) {
                    return { json_token_type::invalid, json_value(nullptr), false, "Invalid number format after decimal point" };
                }
                while (pos_ < source_.size() && std::isdigit(peek_())) {
                    result += peek_();
                    advance_();
                }
            }

            if (peek_() == 'e' || peek_() == 'E') {
                is_float = true;
                result += peek_();
                advance_();
                if (peek_() == '+' || peek_() == '-') {
                    result += peek_();
                    advance_();
                }
                if (pos_ >= source_.size() || !std::isdigit(peek_())) {
                    return { json_token_type::invalid, json_value(nullptr), false, "Invalid exponent format" };
                }
                while (pos_ < source_.size() && std::isdigit(peek_())) {
                    result += peek_();
                    advance_();
                }
            }

            if (is_float) {
                try {
                    json_double_t value = std::stod(result);
                    if (std::isnan(value) || std::isinf(value)) {
                        return { json_token_type::invalid, json_value(nullptr), false, "Invalid float number (NaN or Infinity/Overflow)" };
                    }
                    if (value != 0.0 && std::fpclassify(value) == FP_SUBNORMAL) {
                        return { json_token_type::invalid, json_value(nullptr), false, "Float underflow (subnormal value)" };
                    }
                    return { json_token_type::double_value, json_value(value) };
                }
                catch (const std::exception&) {
                    return { json_token_type::invalid, json_value(nullptr), false, "Invalid float number (parse error or overflow/underflow)" };
                }
            }
            else {
                try {
                    json_int_t value = std::stoll(result);
                    return { json_token_type::int_value, json_value(value) };
                }
                catch (const std::out_of_range&) {
                    return { json_token_type::invalid, json_value(nullptr), false, "Integer overflow/underflow" };
                }
                catch (const std::exception&) {
                    return { json_token_type::invalid, json_value(nullptr), false, "Invalid integer number" };
                }
            }
        }

        json_token parse_string_() {
            std::string result;

            advance_();

            while (pos_ < source_.size()) {
                std::string::value_type ch{ peek_() };

                if (ch == '"') {
                    advance_();
                    return { json_token_type::string_value, json_value(std::move(result)) };
                }

                if (ch == '\\') {
                    advance_();
                    if (pos_ >= source_.size()) {
                        return { json_token_type::invalid, json_value(nullptr),
                                 false, "Unterminated string escape" };
                    }

                    char esc = peek_();
                    switch (esc) {
                        case '"': result += '"'; break;
                        case '\\': result += '\\'; break;
                        case '/': result += '/'; break;
                        case 'b': result += '\b'; break;
                        case 'f': result += '\f'; break;
                        case 'n': result += '\n'; break;
                        case 'r': result += '\r'; break;
                        case 't': result += '\t'; break;
                        case 'u': {
                            advance_();
                            if (pos_ + 4 > source_.size()) {
                                return { json_token_type::invalid, json_value(nullptr),
                                         false, "Incomplete \\u escape sequence" };
                            }
                            std::string hex{ source_.substr(pos_, 4) };
                            for (char c : hex) {
                                if (!std::isxdigit(static_cast<unsigned char>(c))) {
                                    return { json_token_type::invalid, json_value(nullptr),
                                             false, "Invalid hex digit in \\u escape sequence" };
                                }
                            }
                            std::uint16_t code_unit = static_cast<std::uint16_t>(std::stoi(hex, nullptr, 16));
                            advance_(4);

                            std::uint32_t codepoint = 0;
                            if (code_unit >= 0xD800 && code_unit <= 0xDBFF) {
                                if (pos_ + 6 > source_.size() || source_[pos_] != '\\' || source_[pos_ + 1] != 'u') {
                                    return { json_token_type::invalid, json_value(nullptr),
                                             false, "Expected low surrogate after high surrogate" };
                                }
                                advance_(2);
                                std::string hex2{ source_.substr(pos_, 4) };
                                for (char c : hex2) {
                                    if (!std::isxdigit(static_cast<unsigned char>(c))) {
                                        return { json_token_type::invalid, json_value(nullptr),
                                                 false, "Invalid hex digit in low surrogate" };
                                    }
                                }
                                std::uint16_t low = static_cast<std::uint16_t>(std::stoi(hex2, nullptr, 16));
                                if (low < 0xDC00 || low > 0xDFFF) {
                                    return { json_token_type::invalid, json_value(nullptr),
                                             false, "Invalid low surrogate in \\u escape sequence" };
                                }
                                codepoint = 0x10000 + (((code_unit - 0xD800) << 10) | (low - 0xDC00));
                                advance_(4);
                            }
                            else if (code_unit >= 0xDC00 && code_unit <= 0xDFFF) {
                                return { json_token_type::invalid, json_value(nullptr),
                                         false, "Unexpected low surrogate without preceding high surrogate" };
                            }
                            else {
                                codepoint = code_unit;
                            }
                            if (!encode_utf8_(codepoint, result)) {
                                return { json_token_type::invalid, json_value(nullptr),
                                         false, "Invalid Unicode code point in \\u escape sequence" };
                            }
                            continue;
                        }
                        default:
                            return { json_token_type::invalid, json_value(nullptr),
                                     false, "Invalid escape sequence: \\" + std::string(1, esc) };
                    }
                    advance_();
                }
                else {
                    if (static_cast<unsigned char>(ch) < 0x20) {
                        return { json_token_type::invalid, json_value(nullptr),
                                 false, "Control character in string: " + std::string(1, ch) };
                    }
                    result += ch;
                    advance_();
                }
            }

            return { json_token_type::invalid, json_value(nullptr),
                     false, "Unterminated string" };
        }

        inline bool encode_utf8_(std::uint32_t codepoint, std::string& out) {
            if (codepoint <= 0x7F) {
                out += static_cast<char>(codepoint);
                return true;
            }
            if (codepoint <= 0x7FF) {
                out += static_cast<char>(0xC0 | (codepoint >> 6));
                out += static_cast<char>(0x80 | (codepoint & 0x3F));
                return true;
            }
            if (codepoint <= 0xFFFF) {
                out += static_cast<char>(0xE0 | (codepoint >> 12));
                out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                out += static_cast<char>(0x80 | (codepoint & 0x3F));
                return true;
            }
            if (codepoint <= 0x10FFFF) {
                out += static_cast<char>(0xF0 | (codepoint >> 18));
                out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                out += static_cast<char>(0x80 | (codepoint & 0x3F));
                return true;
            }
            return false;
        }

        inline bool starts_with_(const std::string& value) const {
            std::size_t size{ source_.size() };
            if (pos_ + value.size() > size) {
                return false;
            }
            return source_.compare(pos_, value.size(), value) == 0;
        }

        void advance_(std::size_t count = 1) noexcept {
            pos_ += count;
        }

        std::string::value_type peek_() const {
            return source_[pos_];
        }

        void skip_whitespaces_() {
            std::size_t size{ source_.size() };
            while (pos_ < size && std::isspace(static_cast<unsigned char>(source_[pos_]))) {
                pos_++;
            }
        }

    private:

        std::size_t pos_;
        const std::string& source_;

    }; // class json_lexer

    class json_parser {
    public:

        json_parser() = delete;
        json_parser(const json_parser&) = delete;
        json_parser(json_parser&&) = delete;
        json_parser& operator=(const json_parser&) = delete;
        json_parser& operator=(json_parser&&) = delete;

    public:

        explicit json_parser(const std::string& src_str) :
            lexer_{ src_str }
        {

        }

        json_value parse() {

            json_value root;

            std::stack<json_value> stack;
            json_token token = lexer_.next_token();

            if (token.type == json_token_type::end_of_file) {
                log_error_("Empty JSON document");
                return root;
            }
            else if (!token.is_valid || token.type == json_token_type::invalid) {
                log_error_(token.error_str);
                return root;
            }
            else if (is_value_token(token.type)) {
                root = std::move(token.value);
            }
            else if (token.type == json_token_type::left_brace) {
                stack.push(json_value(json_object{}));
                root = parse_complex_(stack);
            }
            else if (token.type == json_token_type::left_bracket) {
                stack.push(json_value(json_array{}));
                root = parse_complex_(stack);
            }
            else {
                log_error_("Unexpected token in root: " + token.error_str);
                return root;
            }

            if (lexer_.next_token().type != json_token_type::end_of_file) {
                log_error_("Unexpected tokens after JSON document end");
                return json_value(nullptr);
            }
            
            return root;
        }

        bool is_valid() const {
            return is_valid_;
        }

        const std::string& error_message() const {
            return error_message_;
        }


    private:

        void log_error_(const std::string& text) {
            is_valid_ = false;
            if (!error_message_.empty()) {
                error_message_ += '\n';
            }
            error_message_ += text;
        }

        json_value parse_complex_(std::stack<json_value>& stack) {
            std::string current_key;
            enum class context_t { object, array } context;
            auto update_context = [&]() {
                context = stack.top().is<json_object>() ? context_t::object : context_t::array;
            };
            update_context();
            enum class token_expected_t {
                value, key, comma, colon
            } token_expected{
                context == context_t::array ? token_expected_t::value : token_expected_t::key
            };
            bool dangling_comma{ false };

            while (true) {
                json_token token = lexer_.next_token();
                if (token.type == json_token_type::left_brace) { // {
                    if (token_expected == token_expected_t::value) {
                        stack.push(json_value(json_object{}));
                        context = context_t::object;
                        token_expected = token_expected_t::key;
                        dangling_comma = false;
                    }
                    else {
                        log_error_("Unexpected left brace in array or object context");
                        return json_value(nullptr);
                    }
                }
                else if (token.type == json_token_type::right_brace) { // }
                    if (dangling_comma && token_expected == token_expected_t::key) {
                        log_error_("Dangling comma before right brace in object context");
                        return json_value(nullptr);
                    }
                    if (context == context_t::object) {
                        json_value complete_node;
                        complete_node = std::move(stack.top());
                        stack.pop();
                        if (stack.empty()) {
                            return json_value(std::move(complete_node));
                        }
                        else {
                            dangling_comma = false;
                            update_context();
                            if (context == context_t::array) {
                                stack.top().as<json_array>().emplace_back(std::move(complete_node));
                            }
                            else {
                                stack.top().as<json_object>()[current_key] = std::move(complete_node);
                            }
                            token_expected = token_expected_t::comma;
                        }
                    }
                    else { // context == context_t::array
                        log_error_("Unexpected right brace in array context");
                        return json_value(nullptr);
                    }
                }
                else if (token.type == json_token_type::left_bracket) { // [
                    if (token_expected == token_expected_t::value) {
                        stack.push(json_value(json_array{}));
                        context = context_t::array;
                        //token_expected = token_expected_t::value;
                        dangling_comma = false;
                    }
                    else {
                        log_error_("Unexpected left bracket in array or object context");
                        return json_value(nullptr);
                    }
                }
                else if (token.type == json_token_type::right_bracket) { // ]
                    if (dangling_comma && token_expected == token_expected_t::value) {
                        log_error_("Dangling comma before right bracket in array context");
                        return json_value(nullptr);
                    }
                    if (context == context_t::array) {
                        json_value complete_node;
                        complete_node = std::move(stack.top());
                        stack.pop();
                        if (stack.empty()) {
                            return json_value(std::move(complete_node));
                        }
                        else {
                            update_context();
                            if (context == context_t::array) {
                                stack.top().as<json_array>().emplace_back(std::move(complete_node));
                            }
                            else {
                                stack.top().as<json_object>()[current_key] = std::move(complete_node);
                            }
                            token_expected = token_expected_t::comma;
                        }
                    }
                    else {
                        log_error_("Unexpected right bracket in object context");
                        return json_value(nullptr);
                    }
                }
                else if (token.type == json_token_type::end_of_file) {
                    log_error_("Unexpected end of file in array or object context");
                    return json_value(nullptr);
                }
                else if (token.type == json_token_type::invalid) {
                    log_error_(token.error_str);
                    return json_value(nullptr);
                }
                else if (is_value_token(token.type)) {
                    if (token_expected == token_expected_t::key && context == context_t::object) {
                        if (!token.value.is<json_string_t>()) {
                            log_error_("Expected string key in object context");
                            return json_value(nullptr);
                        }
                        current_key = std::move(token.value.as<json_string_t>());
                        if (current_key.empty()) {
                            log_error_("Empty key in object context");
                            return json_value(nullptr);
                        }
                        token_expected = token_expected_t::colon;
                    }
                    else if (token_expected == token_expected_t::value) {
                        if (context == context_t::object) {
                            stack.top().as<json_object>()[current_key] = std::move(token.value);
                        }
                        else { // context == context_t::array
                            stack.top().as<json_array>().emplace_back(std::move(token.value));
                        }
                        token_expected = token_expected_t::comma;
                    }
                    else {
                        log_error_("Unexpected value in array or object context");
                        return json_value(nullptr);
                    }
                }
                else if (token.type == json_token_type::comma) {
                    if (token_expected == token_expected_t::comma) {
                        token_expected = context == context_t::array ? token_expected_t::value : token_expected_t::key;
                        dangling_comma = true;
                        continue;
                    }
                    else {
                        log_error_("Unexpected comma in array or object context");
                        return json_value(nullptr);
                    }
                }
                else if (token.type == json_token_type::colon) {
                    if (token_expected == token_expected_t::colon && context == context_t::object) {
                        token_expected = token_expected_t::value;
                        continue;
                    }
                    else {
                        log_error_("Unexpected colon in array or object context");
                        return json_value(nullptr);
                    }
                }
                else {
                    log_error_("Unexpected token in array: " + token.error_str);
                    return json_value(nullptr);
                }
            }

            return json_value(nullptr);
        }

    private:

        json_lexer lexer_;
        bool is_valid_{ true };
        std::string error_message_{};

    }; // class json_parser


    class json_document {
    public:

        json_document() = default;
        json_document(const json_document&) = default;
        json_document(json_document&&) noexcept = default;
        json_document& operator=(const json_document&) = default;
        json_document& operator=(json_document&&) noexcept = default;

    public:

        explicit json_document(json_value root) :
            root_(std::move(root))
        {
        }

        explicit json_document(const std::string& str) {
            from_string(str);
        }

        explicit json_document(std::initializer_list<json_value> list) {
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
            root_ = json_value(nullptr);
        }

        bool is_valid() const {
            return is_valid_;
        }

        const std::string& error_message() const {
            return error_message_;
        }

    public:

        std::string to_string() const {
            return format_value_(root_);
        }

        void from_string(const std::string& str) {
            json_parser parser(str);
            json_value parsed = parser.parse();
            if (parser.is_valid()) {
                root_ = std::move(parsed);
                is_valid_ = true;
                error_message_.clear();
            }
            else {
                is_valid_ = false;
                error_message_ = parser.error_message();
            }
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
        bool is_valid_{ true };
        std::string error_message_{};

    }; // class json_document

} // namespace json