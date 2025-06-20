#pragma once


#include <variant>
#include "concepts.h"

namespace json {


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

        json_value(json_null_t) :
            json_value()
        {
        }

        json_value(json_bool_t value) :
            value_{ value }
        {
        }

        json_value(json_int_t value) :
            value_{ value }
        {
        }

        json_value(json_double_t value) :
            value_{ value }
        {
        }

        json_value(json_string_t value) :
            value_{ std::move(value) }
        {
        }

        json_value(const char* value) :
            json_value(std::string(value))
        {
        }

        json_value(json_array value) :
            value_{ std::move(value) }
        {
        }

        json_value(json_object value) :
            value_{ std::move(value) }
        {
        }

        json_value(value_type value) :
            value_{ std::move(value) }
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
            requires json::concepts::is_json_value<T>
        T& as() {
            return std::get<T>(value_);
        }

        template <typename T>
            requires json::concepts::is_json_value<T>
        const T& as() const {
            return const_cast<json_value*>(this)->as<T>();
        }

        template <typename T>
            requires json::concepts::is_json_value<T>
        T* try_as() {
            return is<T>() ? &std::get<T>(value_) : nullptr;
        }

        template <typename T>
            requires json::concepts::is_json_value<T>
        const T* try_as() const {
            return const_cast<json_value*>(this)->try_as<T>();
        }

        template <typename T>
            requires json::concepts::is_json_value<T>
        bool is() const {
            return std::holds_alternative<T>(value_);
        }

    private:

        value_type value_;

    };


} // namespace json