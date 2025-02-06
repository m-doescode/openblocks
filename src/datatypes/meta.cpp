#include "meta.h"
#include <variant>

Data::String Data::Variant::ToString() const {
    return std::visit([](auto&& it) {
        return it.ToString();
    }, this->wrapped);
}

void Data::Variant::Serialize(pugi::xml_node* node) const {
    std::visit([&](auto&& it) {
        it.Serialize(node);
    }, this->wrapped);
}