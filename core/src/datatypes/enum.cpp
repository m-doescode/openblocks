#include "enum.h"


Enum::Enum(_EnumData* data) : data(data) {}

std::vector<EnumItem> Enum::GetEnumItems() {
    std::vector<EnumItem> enumItems;

    for (int i = 0; i < data->count; i++) {
        enumItems.push_back(EnumItem(data, data->values[i].second, data->values[i].first));
    }

    return enumItems;
}

std::optional<EnumItem> Enum::FromName(std::string name) {
    for (int i = 0; i < data->count; i++) {
        if (data->values[i].second == name)
            return EnumItem(data, name, data->values[i].first);
    }
    return {};
}

std::optional<EnumItem> Enum::FromValue(int value) {
    for (int i = 0; i < data->count; i++) {
        if (data->values[i].first == value)
            return EnumItem(data, data->values[i].second, value);
    }
    return {};
}

EnumItem::EnumItem(_EnumData* parentData, std::string name, int value) : parentData(parentData), name(name), value(value) {}