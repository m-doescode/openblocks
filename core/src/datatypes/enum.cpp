#include "enum.h"
#include "datatypes/base.h"
#include "datatypes/variant.h"
#include "error/data.h"
#include <pugixml.hpp>

TypeMeta::TypeMeta(const Enum* enum_) : descriptor(&EnumItem::TYPE), enum_(enum_) {}

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

EnumItem Enum::FromValueInternal(int value) const {
    auto result = this->FromValue(value);
    if (!result) return EnumItem(data, "", value);
    return result.value();
}

std::string Enum::ToString() const {
    // return "Enum." + this->data->name;
    return this->data->name;
}

bool Enum::operator ==(Enum other) const {
    return this->data == other.data;
}

//

EnumItem::EnumItem(_EnumData* parentData, std::string name, int value) : parentData(parentData), name(name), value(value) {}

std::string EnumItem::ToString() const {
    return "Enum." + parentData->name + "." + name;
}

void EnumItem::Serialize(pugi::xml_node node) const {
    node.set_name("token");
    node.text().set(value);
}

result<EnumItem, DataParseError> EnumItem::Deserialize(pugi::xml_node node, const TypeMeta info) {
    auto result = info.enum_->FromValue(node.text().as_int());
    if (result.has_value()) return result.value();
    return DataParseError(node.text().as_string(), "EnumItem");
}

result<EnumItem, DataParseError> EnumItem::FromString(std::string string, const TypeMeta info) {
    auto result = info.enum_->FromName(string);
    if (result.has_value()) return result.value();
    return DataParseError(string, "EnumItem");
}

bool EnumItem::operator ==(EnumItem other) const {
    return this->parentData == other.parentData && this->value == other.value;
}