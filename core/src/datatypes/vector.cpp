#include "vector.h"
#include <cstdio>
#include <cstdlib>
#include <glm/ext/quaternion_geometric.hpp>
#include <iomanip>
#include <reactphysics3d/mathematics/Vector3.h>
#include <string>
#include <pugixml.hpp>
#include "datatypes/base.h"
#include "datatypes/variant.h"
#include "error/data.h"
#include <sstream>

namespace rp = reactphysics3d;

Vector3::Vector3() : vector(glm::vec3(0, 0, 0)) {};
Vector3::Vector3(const glm::vec3& src) : vector(src) {};
Vector3::Vector3(const rp::Vector3& src) : vector(glm::vec3(src.x, src.y, src.z)) {};
Vector3::Vector3(float x, const float y, float z) : vector(glm::vec3(x, y, z)) {};

Vector3::~Vector3() = default;

Vector3 Vector3::ZERO(0, 0, 0);
Vector3 Vector3::ONE(1, 1, 1);

const std::string Vector3::ToString() const {
    // https://stackoverflow.com/a/46424921/16255372
    std::stringstream stream;
    stream << std::setprecision(8) << std::noshowpoint << X() << ", " << Y() << ", " << Z();
    return stream.str();
}

Vector3::operator glm::vec3() const { return vector; };
Vector3::operator rp::Vector3() const { return rp::Vector3(X(), Y(), Z()); };

// Operators

Vector3 Vector3::operator *(float scale) const {
    return Vector3(this->X() * scale, this->Y() * scale, this->Z() * scale);
}

Vector3 Vector3::operator /(float scale) const {
    return Vector3(this->X() / scale, this->Y() / scale, this->Z() / scale);
}

// Component-wise
Vector3 Vector3::operator *(Vector3 other) const {
    return Vector3(this->X() * other.X(), this->Y() * other.Y(), this->Z() * other.Z());
}

Vector3 Vector3::operator /(Vector3 other) const {
    return Vector3(this->X() / other.X(), this->Y() / other.Y(), this->Z() / other.Z());
}

Vector3 Vector3::operator +(Vector3 other) const {
    return Vector3(this->X() + other.X(), this->Y() + other.Y(), this->Z() + other.Z());
}

Vector3 Vector3::operator -(Vector3 other) const {
    return Vector3(this->X() - other.X(), this->Y() - other.Y(), this->Z() - other.Z());
}

Vector3 Vector3::operator -() const {
    return Vector3(-this->X(), -this->Y(), -this->Z());
}

bool Vector3::operator ==(Vector3 other) const {
    return this->X() == other.X() && this->Y() == other.Y() && this->Z() == other.Z();
}

bool Vector3::operator <(Vector3 other) const {
    return X() < other.X() && Y() < other.Y() && Z() < other.Z();
}

bool Vector3::operator >(Vector3 other) const {
    return X() > other.X() && Y() > other.Y() && Z() > other.Z();
}

Vector3 Vector3::Cross(Vector3 other) const {
    return glm::cross(this->vector, other.vector);
}

float Vector3::Dot(Vector3 other) const {
    return glm::dot(this->vector, other.vector);
}

// Serialization

void Vector3::Serialize(pugi::xml_node node) const {
    node.append_child("X").text().set(std::to_string(this->X()));
    node.append_child("Y").text().set(std::to_string(this->Y()));
    node.append_child("Z").text().set(std::to_string(this->Z()));
}

result<Vector3, DataParseError> Vector3::Deserialize(pugi::xml_node node) {
    return Vector3(node.child("X").text().as_float(), node.child("Y").text().as_float(), node.child("Z").text().as_float());
}

result<Vector3, DataParseError> Vector3::FromString(std::string string) {
    float components[3];

    for (int i = 0; i < 3; i++) {
        if (string.length() == 0) return DataParseError(string, "Vector3");
        while (string[0] == ' ' && string.length() > 0) string.erase(0, 1);
        size_t nextPos = string.find(",");
        if (nextPos == std::string::npos) nextPos = string.length();
        std::string term = string.substr(0, nextPos);
        string.erase(0, nextPos+1);

        char* cpos;
        float value = std::strtof(term.c_str(), &cpos);
        if (cpos == term.c_str()) return DataParseError(string, "Vector3");

        components[i] = value;
    }

    return Vector3(components[0], components[1], components[2]);
}