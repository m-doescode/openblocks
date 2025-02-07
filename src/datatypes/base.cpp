#include "base.h"
#include "meta.h"

#define IMPL_WRAPPER_CLASS(CLASS_NAME, WRAPPED_TYPE, TYPE_NAME) Data::CLASS_NAME::CLASS_NAME(WRAPPED_TYPE in) : value(in) {} \
Data::CLASS_NAME::~CLASS_NAME() = default; \
Data::CLASS_NAME::operator const WRAPPED_TYPE() const { return value; } \
const Data::TypeInfo Data::CLASS_NAME::TYPE = { .name = TYPE_NAME, .deserializer = &Data::CLASS_NAME::Deserialize }; \
const Data::TypeInfo& Data::CLASS_NAME::GetType() const { return Data::CLASS_NAME::TYPE; }; \
void Data::CLASS_NAME::Serialize(pugi::xml_node* node) const { node->text().set(std::string(this->ToString())); }

Data::Base::~Base() {};

Data::Null::Null() {};
Data::Null::~Null() = default;
const Data::TypeInfo Data::Null::TYPE = {
    .name = "null",
    .deserializer = &Data::Null::Deserialize,
};
const Data::TypeInfo& Data::Null::GetType() const { return Data::Null::TYPE; };

const Data::String Data::Null::ToString() const {
    return Data::String("null");
}

void Data::Null::Serialize(pugi::xml_node* node) const {
    node->text().set("null");
}

Data::Variant Data::Null::Deserialize(pugi::xml_node* node) {
    return Data::Null();
}

//

IMPL_WRAPPER_CLASS(Bool, bool, "bool")
IMPL_WRAPPER_CLASS(Int, int, "int")
IMPL_WRAPPER_CLASS(Float, float, "float")
IMPL_WRAPPER_CLASS(String, std::string, "string")

const Data::String Data::Bool::ToString() const {
    return Data::String(value ? "true" : "false");
}

Data::Variant Data::Bool::Deserialize(pugi::xml_node* node) {
    return Data::Bool(node->text().as_bool());
}

const Data::String Data::Int::ToString() const {
    return Data::String(std::to_string(value));
}

Data::Variant Data::Int::Deserialize(pugi::xml_node* node) {
    return Data::Int(node->text().as_int());
}

const Data::String Data::Float::ToString() const {
    return Data::String(std::to_string(value));
}

Data::Variant Data::Float::Deserialize(pugi::xml_node* node) {
    return Data::Float(node->text().as_float());
}

const Data::String Data::String::ToString() const {
    return *this;
}

Data::Variant Data::String::Deserialize(pugi::xml_node* node) {
    return Data::String(node->text().as_string());
}