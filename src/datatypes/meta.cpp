#include "meta.h"
#include <variant>

Data::String Data::Variant::ToString() const {
    return std::visit([](auto&& it) {
        return it.ToString();
    }, this->wrapped);
}