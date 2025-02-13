#include "meta.h"
#include "datatypes/base.h"
#include "datatypes/cframe.h"
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

Data::Variant Data::Variant::Deserialize(pugi::xml_node* node) {
    if (Data::TYPE_MAP.count(node->name()) == 0) {
        fprintf(stderr, "Unknown type for instance: '%s'\n", node->name());
        abort();
    }

    const Data::TypeInfo* type = Data::TYPE_MAP[node->name()];
    return type->deserializer(node);
}

std::map<std::string, const Data::TypeInfo*> Data::TYPE_MAP = {
    { "null", &Data::Null::TYPE },
    { "bool", &Data::Bool::TYPE },
    { "int", &Data::Int::TYPE },
    { "float", &Data::Float::TYPE },
    { "string", &Data::String::TYPE },
    { "Vector3", &Data::Vector3::TYPE },
    { "CoordinateFrame", &Data::CFrame::TYPE },
    { "Color3", &Data::Color3::TYPE },
};