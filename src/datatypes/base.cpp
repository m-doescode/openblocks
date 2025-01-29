#include "base.h"

#define IMPL_WRAPPER_CLASS(CLASS_NAME, WRAPPED_TYPE, TYPE_NAME) Data::CLASS_NAME::CLASS_NAME(WRAPPED_TYPE in) : value(in) {} \
Data::CLASS_NAME::~CLASS_NAME() = default; \
Data::CLASS_NAME::operator WRAPPED_TYPE() { return value; } \
const Data::TypeInfo Data::CLASS_NAME::TYPE = { .name = TYPE_NAME, }; \
const Data::TypeInfo& Data::CLASS_NAME::GetType() const { return Data::CLASS_NAME::TYPE; };

Data::Base::~Base() {};

Data::Null::Null() {};
Data::Null::~Null() = default;
const Data::TypeInfo Data::Null::TYPE = {
    .name = "null",
};
const Data::TypeInfo& Data::Null::GetType() const { return Data::Null::TYPE; };

IMPL_WRAPPER_CLASS(Bool, bool, "bool")
IMPL_WRAPPER_CLASS(Int, int, "int")
IMPL_WRAPPER_CLASS(Float, float, "float")
IMPL_WRAPPER_CLASS(String, std::string, "string")

const Data::String Data::Null::ToString() const {
    return Data::String("null");
}

const Data::String Data::Bool::ToString() const {
    return Data::String(value ? "true" : "false");
}

const Data::String Data::Int::ToString() const {
    return Data::String(std::to_string(value));
}

const Data::String Data::Float::ToString() const {
    return Data::String(std::to_string(value));
}

const Data::String Data::String::ToString() const {
    return *this;
}