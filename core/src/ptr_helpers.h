#pragma once
#include <optional>
#include <memory>

template <typename T>
bool operator ==(std::optional<std::weak_ptr<T>> a, std::optional<std::weak_ptr<T>> b) {
    return (!a.has_value() || a.value().expired()) && (!b.has_value() || b.value().expired())
    || (a.has_value() && !a.value().expired()) && (b.has_value() && !b.value().expired()) && a.value().lock() == b.value().lock();
}

template <typename T>
bool operator ==(std::weak_ptr<T> a, std::weak_ptr<T> b) {
    return a.expired() && b.expired() || (!a.expired() && !b.expired() && a.lock() == b.lock());
}