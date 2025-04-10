#pragma once

#include "error.h"
#include <optional>
#include <string>
#include <variant>

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
    result(Result value);
    result(std::variant<E...> value);

    // Expects the result to be successful, otherwise panic with error message
    Result expect(std::string errMsg = "Unwrapped a result with failure value");

    bool is_success();
    bool is_error();

    std::optional<Result> success();
    std::optional<std::variant<E...>> error();

    // Equivalent to .success
    operator std::optional<Result>();
};