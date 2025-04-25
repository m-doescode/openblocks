#include "color3.h"
#include "meta.h" // IWYU pragma: keep
#include "panic.h"

Data::Color3::Color3(float r, float g, float b) : r(std::clamp(r, 0.f, 1.f)), g(std::clamp(g, 0.f, 1.f)), b(std::clamp(b, 0.f, 1.f)) {};
Data::Color3::Color3(const glm::vec3& vec) : r(std::clamp(vec.x, 0.f, 1.f)), g(std::clamp(vec.y, 0.f, 1.f)), b(std::clamp(vec.z, 0.f, 1.f)) {};

Data::Color3::~Color3() = default;
const Data::TypeInfo Data::Color3::TYPE = {
    .name = "Color3",
    .deserializer = &Data::Color3::Deserialize,
};

const Data::TypeInfo& Data::Color3::GetType() const { return Data::Color3::TYPE; };

const Data::String Data::Color3::ToString() const {
    return std::to_string(int(r*256)) + ", " + std::to_string(int(g*256)) + ", " + std::to_string(int(b*256));
}

Data::Color3::operator glm::vec3() const { return glm::vec3(r, g, b); };

std::string Data::Color3::ToHex() const {
    std::stringstream ss;
    ss << "FF" << std::hex << std::uppercase << std::setfill('0')
        << std::setw(2) << int(r*255)
        << std::setw(2) << int(g*255)
        << std::setw(2) << int(b*255);
    return ss.str();
}


Data::Color3 Data::Color3::FromHex(std::string hex) {
    float r = float(std::stoi(hex.substr(2, 2), nullptr, 16)) / 255;
    float g = float(std::stoi(hex.substr(4, 2), nullptr, 16)) / 255;
    float b = float(std::stoi(hex.substr(6, 2), nullptr, 16)) / 255;
    
    return Color3(r, g, b);
}

// Serialization

void Data::Color3::Serialize(pugi::xml_node node) const {
    node.text().set(this->ToHex());
}

Data::Variant Data::Color3::Deserialize(pugi::xml_node node) {
    return Color3::FromHex(node.text().get());
}


void Data::Color3::PushLuaValue(lua_State* L) const {
    // TODO:
    panic();
}