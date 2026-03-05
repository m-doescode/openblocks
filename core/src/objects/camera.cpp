#include "camera.h"
#include "objectmodel/property.h"
#include "objectmodel/type.h"

INSTANCE_IMPL(Camera)

InstanceType Camera::__buildType() {
    return make_instance_type<Camera>("Camera",
        set_explorer_icon("camera"),

        def_property("CFrame", &Camera::cframe),
        def_property("CameraSubject", &Camera::cameraSubject),
        def_property("FieldOfView", &Camera::fieldOfView)
    );
}

// See also in renderer.cpp
glm::mat4 Camera::getCameraLookAt() {
    // TODO: CFrame math is *completely* broken, this is just a workaround
    return glm::lookAt((glm::vec3)cframe.Position(), (glm::vec3)(cframe.Position() + cframe.LookVector()), (glm::vec3)cframe.UpVector());
}

Vector3 Camera::GetScreenDirection(glm::vec2 screenPos, glm::vec2 screenSize) {
    // VVV Thank goodness for this person's answer
    // https://stackoverflow.com/a/30005258/16255372

    // glm::vec3 worldPos = camera.cameraPos + glm::vec3(glm::vec4(float(position.x()) / width() - 0.5f, float(position.y()) / height() - 0.5f, 0, 0) * camera.getLookAt());
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)screenSize.x / (float)screenSize.y, 0.1f, 100.0f);
    glm::mat4 view = getCameraLookAt();
    glm::mat4 inverseViewport = glm::inverse(projection * view);

    glm::vec2 ndc = glm::vec2(screenPos.x / screenSize.x * 2.f - 1.f, -screenPos.y / screenSize.y * 2.f + 1.f);
    glm::vec4 world = glm::normalize(inverseViewport * glm::vec4(ndc, 1, 1));
    //glm::vec3 flat = glm::vec3(world) / world.w; // https://stackoverflow.com/a/68870587/16255372

    return glm::vec3(world);
}