#include "result.h"
#include <optional>
#include <variant>

template <typename Result, typename ...E>
result<Result, E...>::result(Result value) : value(SuccessContainer { value }) {
}

template <typename Result, typename ...E>
result<Result, E...>::result(std::variant<E...> value) : value(ErrorContainer { value }) {
}

template <typename Result, typename ...E>
bool result<Result, E...>::is_success() {
    return std::holds_alternative<SuccessContainer>(value);
}

template <typename Result, typename ...E>
bool result<Result, E...>::is_error() {
    return std::holds_alternative<ErrorContainer>(value);
}

template <typename Result, typename ...E>
std::optional<Result> result<Result, E...>::success() {
    return std::holds_alternative<SuccessContainer>(value) ? std::make_optional(std::get<SuccessContainer>(value).success) : std::nullopt;
}

template <typename Result, typename ...E>
std::optional<std::variant<E...>> result<Result, E...>::error() {
    return std::holds_alternative<ErrorContainer>(value) ? std::make_optional(std::get<ErrorContainer>(value).error) : std::nullopt;
}

template <typename Result, typename ...E>
result<Result, E...>::operator std::optional<Result>() {
    return this->success();
}
