#include "vector.h"
#include <glm/ext/quaternion_geometric.hpp>

Data::Vector3::Vector3(const glm::vec3& src) : vector(src) {};
Data::Vector3::Vector3(const rp::Vector3& src) : vector(glm::vec3(src.x, src.y, src.z)) {};
Data::Vector3::Vector3(float x, const float y, float z) : vector(glm::vec3(x, y, z)) {};

Data::Vector3::~Vector3() = default;
const Data::TypeInfo Data::Vector3::TYPE = {
    .name = "Vector3",
};

const Data::TypeInfo& Data::Vector3::GetType() const { return Data::Vector3::TYPE; };

Data::Vector3 Data::Vector3::ZERO(0, 0, 0);
Data::Vector3 Data::Vector3::ONE(1, 1, 1);

const Data::String Data::Vector3::ToString() const {
    return std::to_string(X()) + ", " + std::to_string(Y()) + ", " + std::to_string(Z());
}

Data::Vector3::operator glm::vec3() const { return vector; };
Data::Vector3::operator rp::Vector3() const { return rp::Vector3(X(), Y(), Z()); };

// Operators
Data::Vector3 Data::Vector3::operator +(Data::Vector3 other) const {
    return Data::Vector3(this->X() + other.X(), this->Y() + other.Y(), this->Z() + other.Z());
}

Data::Vector3 Data::Vector3::operator -(Data::Vector3 other) const {
    return Data::Vector3(this->X() - other.X(), this->Y() - other.Y(), this->Z() - other.Z());
}

Data::Vector3 Data::Vector3::operator -() const {
    return Data::Vector3(-this->X(), -this->Y(), -this->Z());
}

Data::Vector3 Data::Vector3::Cross(Data::Vector3 other) const {
    return glm::cross(this->vector, other.vector);
}

float Data::Vector3::Dot(Data::Vector3 other) const {
    return glm::dot(this->vector, other.vector);
}

// Serialization

void Data::Vector3::Serialize(pugi::xml_node* node) const {
    node->append_child("X").text().set(std::to_string(this->X()));
    node->append_child("Y").text().set(std::to_string(this->Y()));
    node->append_child("Z").text().set(std::to_string(this->Z()));
}
