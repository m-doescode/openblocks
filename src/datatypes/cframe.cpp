#include "cframe.h"
#include "datatypes/vector.h"
#include "physics/util.h"
#include <glm/ext/matrix_transform.hpp>
#include <reactphysics3d/mathematics/Transform.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

Data::CFrame::CFrame(glm::vec3 translation, glm::mat3 rotation)
    : translation(translation)
    , rotation(rotation) {
}

Data::CFrame::CFrame(Data::Vector3 position, glm::quat quat)
    : translation(position)
    , rotation(quat) {
}

Data::CFrame::CFrame(const rp::Transform& transform) : Data::CFrame::CFrame(rpToGlm(transform.getPosition()), rpToGlm(transform.getOrientation())) {
}

glm::mat3 lookAt(Data::Vector3 position, Data::Vector3 lookAt, Data::Vector3 up) {
    // https://github.com/sgorsten/glm/issues/29#issuecomment-743989030
	Data::Vector3 f = (lookAt - position).Unit(); // Forward/Look
	Data::Vector3 u = up.Unit(); // Up
	Data::Vector3 s = f.Cross(u).Unit(); // Right
	u = s.Cross(u);

	return {
		{ s.X(), u.X(), -f.X() },
		{ s.Y(), u.Y(), -f.Y() },
		{ s.Z(), u.Z(), -f.Z() },
    };
}

Data::CFrame::CFrame(Data::Vector3 position, Data::Vector3 lookAt, Data::Vector3 up)
    : translation(position)
    , rotation(::lookAt(position, lookAt, up)) {
}

Data::CFrame::~CFrame() = default;
const Data::TypeInfo Data::CFrame::TYPE = {
    .name = "CFrame",
};

const Data::TypeInfo& Data::CFrame::GetType() const { return Data::Vector3::TYPE; };

const Data::String Data::CFrame::ToString() const {
    return std::to_string(X()) + ", " + std::to_string(Y()) + ", " + std::to_string(Z());
}

Data::CFrame::operator glm::mat4() const {
    // Always make sure to translate the position first, then rotate. Matrices work backwards
    return glm::translate(glm::mat4(1.0f), this->translation) * glm::mat4(this->rotation);
}

Data::CFrame::operator rp::Transform() const {
    return rp::Transform(glmToRp(translation), glmToRp(rotation));
}

Data::Vector3 Data::CFrame::ToEulerAnglesXYZ() {
    float x;
    float y;
    float z;
    glm::extractEulerAngleXYZ(glm::mat4(this->rotation), x, y, z);
    return Data::Vector3(x, y, z);
}

Data::CFrame Data::CFrame::FromEulerAnglesXYZ(Data::Vector3 vector) {
    glm::mat3 mat = glm::eulerAngleXYZ(vector.X(), vector.Y(), vector.Z());
    return Data::CFrame(Data::Vector3::ZERO, glm::column(mat, 2), (Data::Vector3)glm::column(mat, 1)); // Getting LookAt (3rd) and Up (2nd) vectors
}

// Operators

Data::CFrame Data::CFrame::operator +(Data::Vector3 vector) const {
    return CFrame { this->translation + glm::vec3(vector), this->rotation };
}

Data::CFrame Data::CFrame::operator -(Data::Vector3 vector) const {
    return *this + -vector;
}