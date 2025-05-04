#include "cframe.h"
#include "datatypes/vector.h"
#include "physics/util.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>
#include <reactphysics3d/mathematics/Transform.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
// #include "meta.h" // IWYU pragma: keep

const Data::CFrame Data::CFrame::IDENTITY(glm::vec3(0, 0, 0), glm::mat3(1.f));
const Data::CFrame Data::CFrame::YToZ(glm::vec3(0, 0, 0), glm::mat3(glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0)));

Data::CFrame::CFrame() : Data::CFrame::CFrame(glm::vec3(0, 0, 0), glm::mat3(1.f)) {}

Data::CFrame::CFrame(float x, float y, float z, float R00, float R01, float R02, float R10, float R11, float R12, float R20, float R21, float R22)
    : translation(x, y, z)
    , rotation({
        // { R00, R01, R02 },
        // { R10, R11, R12 },
        // { R20, R21, R22 },
        { R00, R10, R20 },
        { R01, R11, R21 },
        { R02, R12, R22 },
    }) {
}

Data::CFrame::CFrame(glm::vec3 translation, glm::mat3 rotation)
    : translation(translation)
    , rotation(rotation) {
}

Data::CFrame::CFrame(Vector3 position, glm::quat quat)
    : translation(position)
    , rotation(quat) {
}

Data::CFrame::CFrame(const rp::Transform& transform) : Data::CFrame::CFrame(rpToGlm(transform.getPosition()), rpToGlm(transform.getOrientation())) {
}

glm::mat3 lookAt(Vector3 position, Vector3 lookAt, Vector3 up) {
    // https://github.com/sgorsten/linalg/issues/29#issuecomment-743989030
	Vector3 f = (lookAt - position).Unit(); // Forward/Look
	Vector3 u = up.Unit(); // Up
	Vector3 s = f.Cross(u).Unit(); // Right
	u = s.Cross(f).Unit();

	return { s, u, -f };
}

Data::CFrame::CFrame(Vector3 position, Vector3 lookAt, Vector3 up)
    : translation(position)
    , rotation(::lookAt(position, lookAt, up)) {
}

Data::CFrame::~CFrame() = default;

const Data::String Data::CFrame::ToString() const {
    return std::to_string(X()) + ", " + std::to_string(Y()) + ", " + std::to_string(Z());
}

Data::CFrame Data::CFrame::pointToward(Vector3 position, Vector3 toward) {
    return Data::CFrame(position, position + toward, (abs(glm::dot((glm::vec3)toward, glm::vec3(0, 1, 0))) > 0.999) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0));
}

Data::CFrame::operator glm::mat4() const {
    // Always make sure to translate the position first, then rotate. Matrices work backwards
    return glm::translate(glm::mat4(1.0f), this->translation) * glm::mat4(this->rotation);
}

Data::CFrame::operator rp::Transform() const {
    return rp::Transform(glmToRp(translation), glmToRp(rotation));
}

Vector3 Data::CFrame::ToEulerAnglesXYZ() {
    float x;
    float y;
    float z;
    glm::extractEulerAngleXYZ(glm::mat4(this->rotation), x, y, z);
    return Vector3(x, y, z);
}

Data::CFrame Data::CFrame::FromEulerAnglesXYZ(Vector3 vector) {
    glm::mat3 mat = glm::eulerAngleXYZ(vector.X(), vector.Y(), vector.Z());
    return Data::CFrame((glm::vec3)Vector3::ZERO, mat);
}

Data::CFrame Data::CFrame::Inverse() const {
    return CFrame { -translation * glm::transpose(glm::inverse(rotation)), glm::inverse(rotation) };
}


// Operators

Data::CFrame Data::CFrame::operator *(Data::CFrame otherFrame) const {
    return CFrame { this->translation + this->rotation * otherFrame.translation, this->rotation * otherFrame.rotation };
}

Vector3 Data::CFrame::operator *(Vector3 vector) const {
    return this->translation + this->rotation * vector;
}

Data::CFrame Data::CFrame::operator +(Vector3 vector) const {
    return CFrame { this->translation + glm::vec3(vector), this->rotation };
}

Data::CFrame Data::CFrame::operator -(Vector3 vector) const {
    return *this + -vector;
}

// Serialization

void Data::CFrame::Serialize(pugi::xml_node node) const {
    node.append_child("X").text().set(std::to_string(this->X()));
    node.append_child("Y").text().set(std::to_string(this->Y()));
    node.append_child("Z").text().set(std::to_string(this->Z()));
    node.append_child("R00").text().set(std::to_string(this->rotation[0][0]));
    node.append_child("R01").text().set(std::to_string(this->rotation[1][0]));
    node.append_child("R02").text().set(std::to_string(this->rotation[2][0]));
    node.append_child("R10").text().set(std::to_string(this->rotation[0][1]));
    node.append_child("R11").text().set(std::to_string(this->rotation[1][1]));
    node.append_child("R12").text().set(std::to_string(this->rotation[2][1]));
    node.append_child("R20").text().set(std::to_string(this->rotation[0][2]));
    node.append_child("R21").text().set(std::to_string(this->rotation[1][2]));
    node.append_child("R22").text().set(std::to_string(this->rotation[2][2]));
}


Data::Variant Data::CFrame::Deserialize(pugi::xml_node node) {
    return Data::CFrame(
        node.child("X").text().as_float(),
        node.child("Y").text().as_float(),
        node.child("Z").text().as_float(),
        node.child("R00").text().as_float(),
        node.child("R01").text().as_float(),
        node.child("R02").text().as_float(),
        node.child("R10").text().as_float(),
        node.child("R11").text().as_float(),
        node.child("R12").text().as_float(),
        node.child("R20").text().as_float(),
        node.child("R21").text().as_float(),
        node.child("R22").text().as_float()
    );
}