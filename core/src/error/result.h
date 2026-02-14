#pragma once

#include "error/error.h"
#include "logger.h"
#include "panic.h"
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

namespace detail {
    struct __fallible_dummy {};
}

template <typename T_Result, typename ...T_Errors>
class [[nodiscard]] result {
    static_assert(std::conjunction_v<std::is_base_of<Error, T_Errors>...>, "result<T_Errors...> requires T_Errors to derive from Error");

    struct error_state {
        std::variant<T_Errors...> error;
    };

    struct success_state {
        T_Result success;
    };

    std::variant<success_state, error_state> value;
public:
    // Helper for std::variant, etc.
    template <typename ...T_Args, std::enable_if_t<std::is_constructible_v<T_Result, T_Args...>, int> = 0> result(T_Args... args) : value(success_state { T_Result(args...) }) {}
    result(T_Result success) : value(success_state { success }) {}
    result(std::variant<T_Errors...> error) : value(error_state { error }) {}
    template <typename T_Error, std::enable_if_t<std::disjunction_v<std::is_same<T_Error, T_Errors>...>, int> = 0>
    result(T_Error error) : value(error_state { error }) {}

    // Expects the result to be successful, otherwise panic with error message
    T_Result expect(std::string errMsg = "Expected result to contain success") {
        if (isSuccess())
            return std::get<success_state>(value).success;
        std::visit([&](auto&& it) {
            Logger::fatalErrorf("Unwrapped a result with error value: [%s] %s\n\t%s", it.errorType().c_str(), it.message().c_str(), errMsg.c_str());
        }, error().value());
        panic();
    }

    bool isSuccess() { return std::holds_alternative<success_state>(value); }
    bool isError() { return std::holds_alternative<error_state>(value); }

    std::optional<T_Result> success() { return isSuccess() ? std::make_optional(std::get<success_state>(value).success) : std::nullopt; }
    std::optional<std::variant<T_Errors...>> error() { return isError() ? std::make_optional(std::get<error_state>(value).error) : std::nullopt; }

    void logError(Logger::LogLevel logLevel = Logger::LogLevel::ERROR) {
        if (isSuccess()) return;
        std::visit([&](auto&& it) {
            it.logMessage(logLevel);
        }, error().value());
    }

    std::optional<std::string> errorMessage() {
        if (isSuccess()) return std::nullopt;
        return std::visit([&](auto&& it) {
            return it.message();
        }, error().value());
    }

    // Equivalent to .success
    operator std::optional<T_Result>() { return success(); }
    explicit operator bool() const { return isSuccess(); } // Explicity is necessary to prevent auto-casting from result to Variant, for instance
    bool operator !() { return isError(); }
};

template <typename ...T_Errors>
class [[nodiscard]] fallible : public result<detail::__fallible_dummy, T_Errors...> {
public:
    fallible() : result<detail::__fallible_dummy, T_Errors...>(detail::__fallible_dummy {}) {}
    fallible(std::variant<T_Errors...> error) : result<detail::__fallible_dummy, T_Errors...>(error) {}
    template <typename T_Error, std::enable_if_t<std::disjunction_v<std::is_same<T_Error, T_Errors>...>, int> = 0>
    fallible(T_Error error) : result<detail::__fallible_dummy, T_Errors...>(error) {}
};