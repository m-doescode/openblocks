#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <cstdio>
#include <glm/ext.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <utility>
#include <vector>

#include "datatypes/cframe.h"
#include "datatypes/color3.h"
#include "datatypes/vector.h"
#include "handles.h"
#include "math_helper.h"
#include "partassembly.h"
#include "rendering/torus.h"
#include "shader.h"
#include "mesh.h"
#include "defaultmeshes.h"
#include "../camera.h"
#include "../common.h"
#include "../objects/part.h"
#include "skybox.h"
#include "surface.h"
#include "texture3d.h"

#include "renderer.h"

Shader* shader = NULL;
Shader* skyboxShader = NULL;
Shader* handleShader = NULL;
Shader* identityShader = NULL;
Shader* ghostShader = NULL;
Shader* wireframeShader = NULL;
Shader* outlineShader = NULL;
extern Camera camera;
Skybox* skyboxTexture = NULL;
Texture3D* studsTexture = NULL;

bool wireframeRendering = false;

static int viewportWidth, viewportHeight;

void renderInit(GLFWwindow* window, int width, int height) {
    viewportWidth = width, viewportHeight = height;
    glViewport(0, 0, width, height);

    int argc = 1;
    char* argv = const_cast<char*>("");
    initMeshes();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    skyboxTexture = new Skybox({
        "assets/textures/skybox/null_plainsky512_lf.jpg",
        "assets/textures/skybox/null_plainsky512_rt.jpg",
        "assets/textures/skybox/null_plainsky512_up.jpg",
        "assets/textures/skybox/null_plainsky512_dn.jpg",
        "assets/textures/skybox/null_plainsky512_ft.jpg",
        "assets/textures/skybox/null_plainsky512_bk.jpg",
        }, GL_RGB);

    studsTexture = new Texture3D("assets/textures/studs.png", 128, 128, 6, GL_RGBA);

    // Compile shaders
    shader = new Shader("assets/shaders/phong.vs", "assets/shaders/phong.fs");
    skyboxShader = new Shader("assets/shaders/skybox.vs", "assets/shaders/skybox.fs");
    handleShader = new Shader("assets/shaders/handle.vs", "assets/shaders/handle.fs");
    identityShader = new Shader("assets/shaders/identity.vs", "assets/shaders/identity.fs");
    ghostShader = new Shader("assets/shaders/ghost.vs", "assets/shaders/ghost.fs");
    wireframeShader = new Shader("assets/shaders/wireframe.vs", "assets/shaders/wireframe.fs");
    outlineShader = new Shader("assets/shaders/outline.vs", "assets/shaders/outline.fs");
}

void renderParts() {
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use shader
    shader->use();
    // shader->set("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
    // shader->set("lightColor",  glm::vec3(1.0f, 1.0f, 1.0f));

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();
    shader->set("projection", projection);
    shader->set("view", view);
    // shader->set("material", Material {
    //     // .ambient = glm::vec3(1.0f, 0.5f, 0.31f),
    //     .diffuse = glm::vec3(0.639216f, 0.635294f, 0.647059f),
    //     .specular = glm::vec3(0.5f, 0.5f, 0.5f),
    //     .shininess = 16.0f,
    // });
    shader->set("sunLight", DirLight {
        .direction = glm::vec3(-0.2f, -1.0f, -0.3f),
        .ambient = glm::vec3(0.2f, 0.2f, 0.2f),
        .diffuse = glm::vec3(0.5f, 0.5f, 0.5f),
        .specular = glm::vec3(1.0f, 1.0f, 1.0f),
    });
    shader->set("numPointLights", 0);
    // shader->set("pointLights[0]", PointLight {
    //     .position = lightPos,
    //     .ambient = glm::vec3(0.4f, 0.4f, 0.4f),
    //     .diffuse = glm::vec3(1.0f, 1.0f, 1.0f),
    //     .specular = glm::vec3(1.0f, 1.0f, 1.0f),
    //     .constant = 1.0,
    //     .linear = 0.9,
    //     .quadratic = 0.32,
    // });
    studsTexture->activate(0);
    shader->set("studs", 0);

    // Pre-calculate the normal matrix for the shader

    // Pass in the camera position
    shader->set("viewPos", camera.cameraPos);

    // Sort by nearest
    std::map<float, std::shared_ptr<Part>> sorted;
    for (auto it = gWorkspace()->GetDescendantsStart(); it != gWorkspace()->GetDescendantsEnd(); it++) {
        std::shared_ptr<Instance> inst = *it;
        if (inst->GetClass()->className != "Part") continue;
        std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(inst);
        if (part->transparency > 0.00001) {
            float distance = glm::length(glm::vec3(Vector3(camera.cameraPos) - part->position()));
            sorted[distance] = part;
        } else {
            glm::mat4 model = part->cframe;
            // if (part->name == "camera") model = camera.getLookAt();
            model = glm::scale(model, part->size);
            shader->set("model", model);
            shader->set("material", Material {
                .diffuse = part->color,
                .specular = glm::vec3(0.5f, 0.5f, 0.5f),
                .shininess = 16.0f,
            });
            glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
            shader->set("normalMatrix", normalMatrix);
            shader->set("texScale", part->size);
            shader->set("transparency", part->transparency);

            shader->set("surfaces[" + std::to_string(NormalId::Right) + "]", part->rightSurface);
            shader->set("surfaces[" + std::to_string(NormalId::Top) + "]", part->topSurface);
            shader->set("surfaces[" + std::to_string(NormalId::Back) + "]", part->backSurface);
            shader->set("surfaces[" + std::to_string(NormalId::Left) + "]", part->leftSurface);
            shader->set("surfaces[" + std::to_string(NormalId::Bottom) + "]", part->bottomSurface);
            shader->set("surfaces[" + std::to_string(NormalId::Front) + "]", part->frontSurface);

            CUBE_MESH->bind();
            glDrawArrays(GL_TRIANGLES, 0, CUBE_MESH->vertexCount);
        }
    }

    // TODO: Same as todo in src/physics/simulation.cpp
    // According to LearnOpenGL, std::map automatically sorts its contents.
    for (std::map<float, std::shared_ptr<Part>>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); it++) {
        std::shared_ptr<Part> part = it->second;
        glm::mat4 model = part->cframe;
        // if (part->name == "camera") model = camera.getLookAt();
        model = glm::scale(model, part->size);
        shader->set("model", model);
        shader->set("material", Material {
            .diffuse = part->color,
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 16.0f,
        });
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        shader->set("normalMatrix", normalMatrix);
        shader->set("texScale", part->size);
        shader->set("transparency", part->transparency);

        shader->set("surfaces[" + std::to_string(NormalId::Right) + "]", part->rightSurface);
        shader->set("surfaces[" + std::to_string(NormalId::Top) + "]", part->topSurface);
        shader->set("surfaces[" + std::to_string(NormalId::Back) + "]", part->backSurface);
        shader->set("surfaces[" + std::to_string(NormalId::Left) + "]", part->leftSurface);
        shader->set("surfaces[" + std::to_string(NormalId::Bottom) + "]", part->bottomSurface);
        shader->set("surfaces[" + std::to_string(NormalId::Front) + "]", part->frontSurface);

        CUBE_MESH->bind();
        glDrawArrays(GL_TRIANGLES, 0, CUBE_MESH->vertexCount);
    }
}

static Vector3 FACES[6] = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1},
    {-1, 0, 0},
    {0, -1, 0},
    {0, 0, -1},
};

void renderSurfaceExtras() {
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use shader
    ghostShader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();
    ghostShader->set("projection", projection);
    ghostShader->set("view", view);
    ghostShader->set("color", glm::vec3(0.87f, 0.87f, 0.0f));

    // Pass in the camera position
    ghostShader->set("viewPos", camera.cameraPos);

    for (auto it = gWorkspace()->GetDescendantsStart(); it != gWorkspace()->GetDescendantsEnd(); it++) {
        std::shared_ptr<Instance> inst = *it;
        if (!inst->IsA("Part")) continue;
        std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(inst);
        for (int i = 0; i < 6; i++) {
            NormalId face = (NormalId)i;
            SurfaceType type = part->GetSurfaceFromFace(face);
            if (type <= SurfaceType::SurfaceUniversal) continue;

            Vector3 surfaceCenter = part->cframe * (normalFromFace(face) * part->size / 2.f);

            glm::mat4 model = CFrame::pointToward(surfaceCenter, part->cframe.Rotation() * normalFromFace(face));
            model = glm::scale(model, glm::vec3(0.4,0.4,0.4));
            ghostShader->set("model", model);
            glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    
            CYLINDER_MESH->bind();
            glDrawArrays(GL_TRIANGLES, 0, CYLINDER_MESH->vertexCount);
        }
    }
}

void renderSkyBox() {
    glDepthMask(GL_FALSE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    skyboxShader->use();

    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    // Remove translation component of view, making us always at (0, 0, 0)
    glm::mat4 view = glm::mat4(glm::mat3(camera.getLookAt()));

    skyboxShader->set("projection", projection);
    skyboxShader->set("view", view);

    skyboxShader->set("uTexture", 0);

    CUBE_MESH->bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

static CFrame XYZToZXY(glm::vec3(0, 0, 0), -glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

void renderHandles() {
    if (!editorToolHandles.active) return;

    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // This is right... Probably.....
    
    // Use shader
    handleShader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();
    handleShader->set("projection", projection);
    handleShader->set("view", view);
    handleShader->set("sunLight", DirLight {
        .direction = glm::vec3(-0.2f, -1.0f, -0.3f),
        .ambient = glm::vec3(0.2f, 0.2f, 0.2f),
        .diffuse = glm::vec3(0.5f, 0.5f, 0.5f),
        .specular = glm::vec3(1.0f, 1.0f, 1.0f),
    });
    handleShader->set("numPointLights", 0);

    // Pass in the camera position
    handleShader->set("viewPos", camera.cameraPos);

    for (auto face : HandleFace::Faces) {
        glm::mat4 model = getHandleCFrame(face);
        model = glm::scale(model, (glm::vec3)handleSize(face));
        handleShader->set("model", model);
        handleShader->set("material", Material {
            .diffuse = editorToolHandles.handlesType == HandlesType::RotateHandles ? (glm::vec3)(XYZToZXY * glm::abs(face.normal)) : glm::abs(face.normal),
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 16.0f,
        });
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        handleShader->set("normalMatrix", normalMatrix);

        if (editorToolHandles.handlesType == HandlesType::MoveHandles) {
            ARROW_MESH->bind();
            glDrawArrays(GL_TRIANGLES, 0, ARROW_MESH->vertexCount);
        } else {
            SPHERE_MESH->bind();
            glDrawArrays(GL_TRIANGLES, 0, SPHERE_MESH->vertexCount);
        }
    }

    // 2d square overlay
    glDisable(GL_CULL_FACE);

    identityShader->use();
    identityShader->set("aColor", glm::vec3(0.f, 1.f, 1.f));
    
    for (auto face : HandleFace::Faces) {
        CFrame cframe = getHandleCFrame(face);
        glm::vec4 screenPos = projection * view * glm::vec4((glm::vec3)cframe.Position(), 1.0f);

        if (screenPos.z < 0) continue;
        glm::vec3 ndcCoords = screenPos / screenPos.w;

        float rad = 5;
        float xRad = rad * 1/viewportWidth;
        float yRad = rad * 1/viewportHeight;

        glBegin(GL_QUADS);
            glVertex3f(ndcCoords.x - xRad, ndcCoords.y - yRad, 0);
            glVertex3f(ndcCoords.x + xRad, ndcCoords.y - yRad, 0);
            glVertex3f(ndcCoords.x + xRad, ndcCoords.y + yRad, 0);
            glVertex3f(ndcCoords.x - xRad, ndcCoords.y + yRad, 0);
        glEnd();
    }
}

void renderAABB() {
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use shader
    ghostShader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();
    ghostShader->set("projection", projection);
    ghostShader->set("view", view);

    // Pass in the camera position
    ghostShader->set("viewPos", camera.cameraPos);

    ghostShader->set("transparency", 0.5f);
    ghostShader->set("color", glm::vec3(1.f, 0.f, 0.f));

    // Sort by nearest
    for (std::shared_ptr<Instance> inst : gWorkspace()->GetChildren()) {
        if (inst->GetClass()->className != "Part") continue;
        std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(inst);
        glm::mat4 model = CFrame::IDENTITY + part->cframe.Position();
        printf("AABB is supposedly (%f, %f, %f)\n", part->GetAABB().X(), part->GetAABB().Y(), part->GetAABB().Z());
        model = glm::scale(model, (glm::vec3)part->GetAABB());
        ghostShader->set("model", model);
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        ghostShader->set("normalMatrix", normalMatrix);

        CUBE_MESH->bind();
        glDrawArrays(GL_TRIANGLES, 0, CUBE_MESH->vertexCount);
    }
}

void renderWireframe() {
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    // Use shader
    wireframeShader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();
    wireframeShader->set("projection", projection);
    wireframeShader->set("view", view);

    // Pass in the camera position
    wireframeShader->set("viewPos", camera.cameraPos);

    wireframeShader->set("transparency", 0.5f);
    wireframeShader->set("color", glm::vec3(1.f, 0.f, 0.f));

    // Sort by nearest
    for (std::shared_ptr<Instance> inst : gWorkspace()->GetChildren()) {
        if (inst->GetClass()->className != "Part") continue;
        std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(inst);
        glm::mat4 model = part->cframe;
        model = glm::scale(model, (glm::vec3)part->size);
        wireframeShader->set("model", model);
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        wireframeShader->set("normalMatrix", normalMatrix);

        CUBE_MESH->bind();
        glDrawArrays(GL_TRIANGLES, 0, CUBE_MESH->vertexCount);
    }

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

void renderOutlines() {
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use shader
    outlineShader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();
    outlineShader->set("projection", projection);
    outlineShader->set("view", view);

    // Pass in the camera position
    outlineShader->set("viewPos", camera.cameraPos);
    outlineShader->set("thickness", 0.4f);

    outlineShader->set("color", glm::vec3(0.204, 0.584, 0.922));

    glm::vec3 min, max;
    bool first = true;
    int count = 0;

    for (auto it = gWorkspace()->GetDescendantsStart(); it != gWorkspace()->GetDescendantsEnd(); it++) {
        std::shared_ptr<Instance> inst = *it;
        if (inst->GetClass() != &Part::TYPE) continue;
        std::shared_ptr<Part> part = std::dynamic_pointer_cast<Part>(inst);
        if (!part->selected) continue;

        count++;
        if (first)
            min = part->position(), max = part->position();
        first = false;

        Vector3 aabbSize = part->GetAABB();
        expandAABB(min, max, part->position() - aabbSize / 2.f);
        expandAABB(min, max, part->position() + aabbSize / 2.f);

        glm::mat4 model = part->cframe;
        model = glm::scale(model, (glm::vec3)part->size + glm::vec3(0.2));
        outlineShader->set("model", model);
        outlineShader->set("scale", part->size + glm::vec3(0.1));

        OUTLINE_MESH->bind();
        glDrawArrays(GL_TRIANGLES, 0, OUTLINE_MESH->vertexCount);
    }

    // Render AABB of selected parts
    if (count <= 1) return;
    
    glm::vec3 outlineSize, outlinePos;
    outlineSize = (max - min);
    outlinePos = (max + min) / 2.f;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), outlinePos);
    model = glm::scale(model, outlineSize + glm::vec3(0.1));
    outlineShader->set("model", model);
    outlineShader->set("scale", outlineSize + glm::vec3(0.05));
    outlineShader->set("thickness", 0.2f);

    OUTLINE_MESH->bind();
    glDrawArrays(GL_TRIANGLES, 0, OUTLINE_MESH->vertexCount);
}

void renderSelectionAssembly() {
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    PartAssembly selectionAssembly = PartAssembly::FromSelection();

    // Use shader
    outlineShader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();
    outlineShader->set("projection", projection);
    outlineShader->set("view", view);

    // Pass in the camera position
    outlineShader->set("viewPos", camera.cameraPos);
    outlineShader->set("thickness", 0.4f);

    outlineShader->set("color", glm::vec3(1.f, 0.f, 0.f));

    glm::mat4 model = selectionAssembly.assemblyOrigin();
    model = glm::scale(model, (glm::vec3)selectionAssembly.bounds() + glm::vec3(0.1));
    outlineShader->set("model", model);
    outlineShader->set("scale", (glm::vec3)selectionAssembly.bounds() + glm::vec3(0.05));
    outlineShader->set("thickness", 0.2f);

    OUTLINE_MESH->bind();
    glDrawArrays(GL_TRIANGLES, 0, OUTLINE_MESH->vertexCount);
}

void renderRotationArcs() {
    if (!editorToolHandles.active || editorToolHandles.handlesType != HandlesType::RotateHandles) return;

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use shader
    handleShader->use();

    handleShader->set("sunLight", DirLight {
        .direction = glm::vec3(-0.2f, -1.0f, -0.3f),
        .ambient = glm::vec3(0.2f, 0.2f, 0.2f),
        .diffuse = glm::vec3(0.5f, 0.5f, 0.5f),
        .specular = glm::vec3(1.0f, 1.0f, 1.0f),
    });

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();
    handleShader->set("projection", projection);
    handleShader->set("view", view);

    // Pass in the camera position
    handleShader->set("viewPos", camera.cameraPos);

    PartAssembly assembly = PartAssembly::FromSelection();

    for (HandleFace face : HandleFace::Faces) {
        if (glm::any(glm::lessThan(face.normal, glm::vec3(0)))) continue;
        glm::mat4 model = assembly.assemblyOrigin() * CFrame(glm::vec3(0), face.normal, glm::vec3(0, 1.01, 0.1));
        handleShader->set("model", model);

        float radius = glm::max(assembly.bounds().X(), assembly.bounds().Y(), assembly.bounds().Z()) / 2.f + 2.f;

        handleShader->set("material", Material {
            .diffuse = glm::abs(face.normal),
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 16.0f,
        });
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        handleShader->set("normalMatrix", normalMatrix);

        genTorus(radius, 0.05f, 20, 20);
    }
}

std::vector<std::pair<CFrame, Color3>> DEBUG_CFRAMES;

void renderDebugCFrames() {
    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // This is right... Probably.....
    
    // Use shader
    handleShader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();
    handleShader->set("projection", projection);
    handleShader->set("view", view);
    handleShader->set("sunLight", DirLight {
        .direction = glm::vec3(-0.2f, -1.0f, -0.3f),
        .ambient = glm::vec3(0.2f, 0.2f, 0.2f),
        .diffuse = glm::vec3(0.5f, 0.5f, 0.5f),
        .specular = glm::vec3(1.0f, 1.0f, 1.0f),
    });
    handleShader->set("numPointLights", 0);

    // Pass in the camera position
    handleShader->set("viewPos", camera.cameraPos);

    for (auto& [frame, color] : DEBUG_CFRAMES) {
        glm::mat4 model = frame;
        model = glm::scale(model, glm::vec3(0.5, 0.5, 1.5));
        handleShader->set("model", model);
        handleShader->set("material", Material {
            .diffuse = color,
            .specular = glm::vec3(0.5f, 0.5f, 0.5f),
            .shininess = 16.0f,
        });
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
        handleShader->set("normalMatrix", normalMatrix);

        ARROW_MESH->bind();
        glDrawArrays(GL_TRIANGLES, 0, ARROW_MESH->vertexCount);
    }
}

void addDebugRenderCFrame(CFrame frame) {
    addDebugRenderCFrame(frame, Color3(0, 0, 1));
}

void addDebugRenderCFrame(CFrame frame, Color3 color) {
    DEBUG_CFRAMES.push_back(std::make_pair(frame, color));
}

void render(GLFWwindow* window) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderSkyBox();
    renderHandles();
    renderDebugCFrames();
    renderParts();
    renderSurfaceExtras();
    renderOutlines();
    // renderSelectionAssembly();
    renderRotationArcs();
    if (wireframeRendering)
        renderWireframe();
    // TODO: Make this a debug flag
    // renderAABB();
}

void setViewport(int width, int height) {
    viewportWidth = width, viewportHeight = height;
}