#include "enum.h"
#include "datatypes/base.h"
#include "datatypes/variant.h"

TypeInfo::TypeInfo(Enum* enum_) : enum_(enum_), descriptor(&EnumItem::TYPE) {}

Enum::Enum(_EnumData* data) : data(data) {}

std::vector<EnumItem> Enum::GetEnumItems() const {
    std::vector<EnumItem> enumItems;

    for (int i = 0; i < data->count; i++) {
        enumItems.push_back(EnumItem(data, data->values[i].second, data->values[i].first));
    }

    return enumItems;
}

std::optional<EnumItem> Enum::FromName(std::string name) const {
    for (int i = 0; i < data->count; i++) {
        if (data->values[i].second == name)
            return EnumItem(data, name, data->values[i].first);
    }
    return {};
}

std::optional<EnumItem> Enum::FromValue(int value) const {
    for (int i = 0; i < data->count; i++) {
        if (data->values[i].first == value)
            return EnumItem(data, data->values[i].second, value);
    }
    return {};
}

EnumItem::EnumItem(_EnumData* parentData, std::string name, int value) : parentData(parentData), name(name), value(value) {}

//

std::string Enum::ToString() {
    return "Enum." + this->data->name;
}

void Enum::PushLuaValue(lua_State*) {

}

Variant Enum::FromLuaValue(lua_State*, int) {
    
}

const TypeDescriptor Enum::TYPE {
    .name = "Enum",
    .toString = toVariantFunction(&Enum::ToString),
    // .fromString = Enum_FromString,
    // .pushLuaValue = &Enum_PushLuaValue,
    .fromLuaValue = toVariantGenerator(Enum::FromLuaValue),
    // .serializer = Enum_Serialize,
    // .deserializer = Enum_Deserialize,
};