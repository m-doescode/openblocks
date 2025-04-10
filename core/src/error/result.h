#pragma once

#include "error.h"
#include "logger.h"
#include "panic.h"
#include <optional>
#include <string>
#include <variant>

struct DUMMY_VALUE {};

template <typename Result, typename ...E>
class [[nodiscard]] result {
    struct ErrorContainer {
        std::variant<E...> error;
    };

    struct SuccessContainer {
        Result success;
    };

    std::variant<SuccessContainer, ErrorContainer> value;
public:
    result(Result success) : value(SuccessContainer { success }) {}
    result(std::variant<E...> error) : value(ErrorContainer { error }) {}

    // Expects the result to be successful, otherwise panic with error message
    Result expect(std::string errMsg = "Unwrapped a result with failure value") {
        if (is_success())
            return std::get<SuccessContainer>(value).success;
        Logger::fatalError(errMsg);
        panic();
    }

    bool is_success() { return std::holds_alternative<SuccessContainer>(value); }
    bool is_error() { return std::holds_alternative<ErrorContainer>(value); }

    std::optional<Result> success() { return is_success() ? std::get<SuccessContainer>(value).success : std::nullopt; }
    std::optional<std::variant<E...>> error() { return is_error() ? std::make_optional(std::get<ErrorContainer>(value).error) : std::nullopt; }

    void logError(Logger::LogLevel logLevel = Logger::LogLevel::ERROR) {
        if (is_success()) return;
        std::visit([&](auto&& it) {
            it.logMessage(logLevel);
        }, error().value());
    }

    // Equivalent to .success
    operator std::optional<Result>() { return success(); }
    operator bool() { return is_success(); }
    bool operator !() { return is_error(); }
};

template <typename ...E>
class fallible : public result<DUMMY_VALUE, E...> {
public:
    fallible() : result<DUMMY_VALUE, E...>(DUMMY_VALUE {}) {}
    fallible(std::variant<E...> error) : result<DUMMY_VALUE, E...>(error) {}
};