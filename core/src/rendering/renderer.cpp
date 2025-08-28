#include <glad/gl.h>
#include <cmath>
#include <cstdio>
#include <glm/ext.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <utility>
#include <vector>

#include "datatypes/cframe.h"
#include "datatypes/color3.h"
#include "datatypes/vector.h"
#include "enum/part.h"
#include "handles.h"
#include "math_helper.h"
#include "objects/hint.h"
#include "objects/message.h"
#include "objects/part/wedgepart.h"
#include "objects/service/selection.h"
#include "partassembly.h"
#include "rendering/font.h"
#include "rendering/mesh2d.h"
#include "rendering/texture.h"
#include "rendering/torus.h"
#include "shader.h"
#include "mesh.h"
#include "defaultmeshes.h"
#include "camera.h"
#include "common.h"
#include "objects/part/part.h"
#include "skybox.h"
#include "enum/surface.h"
#include "texture3d.h"
#include "timeutil.h"

#include "renderer.h"

Shader* shader = NULL;
Shader* skyboxShader = NULL;
Shader* handleShader = NULL;
Shader* identityShader = NULL;
Shader* ghostShader = NULL;
Shader* wireframeShader = NULL;
Shader* outlineShader = NULL;
Shader* debugFontShader = NULL;
Shader* generic2dShader = NULL;
extern Camera camera;
Skybox* skyboxTexture = NULL;
Texture3D* studsTexture = NULL;
Texture* debugFontTexture = NULL;
Mesh2D* rect2DMesh = NULL;

std::shared_ptr<Font> sansSerif;

bool debugRendererEnabled = false;
bool wireframeRendering = false;

int viewportWidth, viewportHeight;

void renderDebugInfo();
void drawRect(int x, int y, int width, int height, glm::vec4 color);
inline void drawRect(int x, int y, int width, int height, glm::vec3 color) { return drawRect(x, y, width, height, glm::vec4(color, 1)); };

void renderInit(int width, int height) {
    viewportWidth = width, viewportHeight = height;
    glViewport(0, 0, width, height);

    initMeshes();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glFrontFace(GL_CW);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    debugFontTexture = new Texture("assets/textures/debugfnt.bmp", GL_RGB);

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
    debugFontShader = new Shader("assets/shaders/debug/debugfont.vs", "assets/shaders/debug/debugfont.fs");
    generic2dShader = new Shader("assets/shaders/generic2d.vs", "assets/shaders/generic2d.fs");

    // Create mesh for 2d rectangle
    float rectVerts[] = {
        0.0, 0.0,    0.0, 0.0,
        1.0, 0.0,    1.0, 0.0,
        1.0, 1.0,    1.0, 1.0,

        1.0, 1.0,    1.0, 1.0,
        0.0, 1.0,    0.0, 1.0,
        0.0, 0.0,    0.0, 0.0,
    };

    rect2DMesh = new Mesh2D(6, rectVerts);

    // Initialize fonts
    fontInit();
    sansSerif = loadFont("LiberationSans-Regular.ttf");
}

static void renderPart(std::shared_ptr<BasePart> part) {
    glm::mat4 model = part->cframe;
    Vector3 size = part->GetEffectiveSize();
    model = glm::scale(model, (glm::vec3)size);
    shader->set("model", model);
    shader->set("material", Material {
        .diffuse = part->color,
        .specular = glm::vec3(0.5f, 0.5f, 0.5f),
        .shininess = 16.0f,
    });
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    shader->set("normalMatrix", normalMatrix);
    shader->set("texScale", size);
    shader->set("transparency", part->transparency);
    shader->set("reflectance", part->reflectance);

    shader->set("surfaces[" + std::to_string(NormalId::Right) + "]", (int)part->rightSurface);
    shader->set("surfaces[" + std::to_string(NormalId::Top) + "]", (int)part->topSurface);
    shader->set("surfaces[" + std::to_string(NormalId::Back) + "]", (int)part->backSurface);
    shader->set("surfaces[" + std::to_string(NormalId::Left) + "]", (int)part->leftSurface);
    shader->set("surfaces[" + std::to_string(NormalId::Bottom) + "]", (int)part->bottomSurface);
    shader->set("surfaces[" + std::to_string(NormalId::Front) + "]", (int)part->frontSurface);

    PartType shape = part->IsA<Part>() ? part->CastTo<Part>().expect()->shape : PartType::Block;
    if (part->IsA<WedgePart>()) {
        WEDGE_MESH->bind();
        glDrawArrays(GL_TRIANGLES, 0, WEDGE_MESH->vertexCount);
    } else if (shape == PartType::Ball) { // Part
        SPHERE_MESH->bind();
        glDrawArrays(GL_TRIANGLES, 0, SPHERE_MESH->vertexCount);
    } else if (shape == PartType::Block) {
        glFrontFace(GL_CW);
        CUBE_MESH->bind();
        glDrawArrays(GL_TRIANGLES, 0, CUBE_MESH->vertexCount);
    }
}

void renderParts() {
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use shader
    shader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)viewportWidth / (float)viewportHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.getLookAt();
    shader->set("projection", projection);
    shader->set("view", view);
    shader->set("sunLight", DirLight {
        .direction = glm::vec3(-0.2f, -1.0f, -0.3f),
        .ambient = glm::vec3(0.2f, 0.2f, 0.2f),
        .diffuse = glm::vec3(0.5f, 0.5f, 0.5f),
        .specular = glm::vec3(1.0f, 1.0f, 1.0f),
    });
    shader->set("numPointLights", 0);
    studsTexture->activate(0);
    shader->set("studs", 0);
    skyboxTexture->activate(1);
    shader->set("skybox", 1);

    // Pre-calculate the normal matrix for the shader

    // Pass in the camera position
    shader->set("viewPos", camera.cameraPos);
    

    // Sort by nearest
    std::map<float, std::shared_ptr<BasePart>> sorted;
    for (auto it = gWorkspace()->GetDescendantsStart(); it != gWorkspace()->GetDescendantsEnd(); it++) {
        if (!it->IsA<BasePart>()) continue;
        std::shared_ptr<BasePart> part = std::dynamic_pointer_cast<BasePart>(*it);

        if (part->transparency > 0.00001) {
            float distance = glm::length(glm::vec3(Vector3(camera.cameraPos) - part->position()));
            sorted[distance] = part;
        } else {
            renderPart(part);
        }
    }

    // TODO: Same as todo in src/physics/simulation.cpp
    // According to LearnOpenGL, std::map automatically sorts its contents.
    for (std::map<float, std::shared_ptr<BasePart>>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); it++) {
        std::shared_ptr<BasePart> part = it->second;
        renderPart(part);
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
        std::shared_ptr<BasePart> part = std::dynamic_pointer_cast<BasePart>(inst);
        for (int i = 0; i < 6; i++) {
            NormalId face = (NormalId)i;
            SurfaceType type = part->GetSurfaceFromFace(face);
            if (type <= SurfaceType::Universal) continue;

            Vector3 surfaceCenter = part->cframe * (normalFromFace(face) * part->size / 2.f);

            glm::mat4 model = CFrame::pointToward(surfaceCenter, part->cframe.Rotation() * normalFromFace(face));
            model = glm::scale(model, glm::vec3(0.4,0.4,0.4));
            ghostShader->set("model", model);
    
            CYLINDER_MESH->bind();
            glDrawArrays(GL_TRIANGLES, 0, CYLINDER_MESH->vertexCount);
        }
    }
}

void renderSkyBox() {
    glDepthMask(GL_FALSE);
    glCullFace(GL_FRONT);

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

    auto assembly = PartAssembly::FromSelection(gDataModel->GetService<Selection>());
    if (assembly.size() == Vector3::ZERO) return;

    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);
    
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
    for (auto face : HandleFace::Faces) {
        CFrame cframe = getHandleCFrame(face);

        glm::vec4 screenPos = projection * view * glm::vec4((glm::vec3)cframe.Position(), 1.0f);
        screenPos /= screenPos.w;
        screenPos += 1; screenPos /= 2; screenPos.y = 1 - screenPos.y; screenPos *= glm::vec4(glm::vec2(viewportWidth, viewportHeight), 1, 1);
        
        drawRect(screenPos.x - 3, screenPos.y - 3, 6, 6, glm::vec3(0, 1, 1));
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
        std::shared_ptr<BasePart> part = std::dynamic_pointer_cast<BasePart>(inst);
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
        std::shared_ptr<BasePart> part = std::dynamic_pointer_cast<BasePart>(inst);
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

    std::shared_ptr<Selection> selection = gDataModel->GetService<Selection>();
    for (auto inst : selection->Get()) {
        if (inst->GetClass() != &BasePart::TYPE) continue;
        std::shared_ptr<BasePart> part = std::dynamic_pointer_cast<BasePart>(inst);

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
    PartAssembly selectionAssembly = PartAssembly::FromSelection(gDataModel->GetService<Selection>());
    if (selectionAssembly.size() == Vector3()) return;
    glm::vec3 outlineSize = selectionAssembly.bounds();
    glm::vec3 outlinePos = selectionAssembly.assemblyOrigin().Position();

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    PartAssembly selectionAssembly = PartAssembly::FromSelection(gDataModel->GetService<Selection>());

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
    model = glm::scale(model, (glm::vec3)selectionAssembly.size() + glm::vec3(0.1));
    outlineShader->set("model", model);
    outlineShader->set("scale", (glm::vec3)selectionAssembly.size() + glm::vec3(0.05));
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

    PartAssembly assembly = PartAssembly::FromSelection(gDataModel->GetService<Selection>());
    if (assembly.size() == Vector3::ZERO) return; // No parts are selected

    for (HandleFace face : HandleFace::Faces) {
        if (glm::any(glm::lessThan(face.normal, glm::vec3(0)))) continue;
        glm::mat4 model = assembly.assemblyOrigin() * CFrame(glm::vec3(0), face.normal, glm::vec3(0, 1.01, 0.1));
        handleShader->set("model", model);

        float radius = glm::max(assembly.size().X(), assembly.size().Y(), assembly.size().Z()) / 2.f + 2.f;

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

void renderMessages() {
    glDisable(GL_DEPTH_TEST);
    // glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    for (auto it = gWorkspace()->GetDescendantsStart(); it != gWorkspace()->GetDescendantsEnd(); it++) {
        if (!it->IsA<Message>()) continue;
        std::shared_ptr<Message> message = it->CastTo<Message>().expect();

        float textWidth = calcTextWidth(sansSerif, message->text);

        // Render hint
        if (message->GetClass() == &Hint::TYPE) {
            drawRect(0, 0, viewportWidth, 20, glm::vec4(0,0,0,1));
            drawText(sansSerif, message->text, (viewportWidth - textWidth) / 2, 0);
        } else {
            // Don't draw if text is empty
            if (message->text == "") continue;

            float strokedTextWidth = calcTextWidth(sansSerif, message->text, true);
            drawRect(0, 0, viewportWidth, viewportHeight, glm::vec4(0.5));
            drawText(sansSerif, message->text, ((float)viewportWidth - textWidth) / 2, ((float)viewportHeight - sansSerif->height) / 2, 1.f, glm::vec3(0), true);
            drawText(sansSerif, message->text, ((float)viewportWidth - strokedTextWidth) / 2, ((float)viewportHeight - sansSerif->height) / 2, 1.f, glm::vec3(1), false);
        }
    }
}

tu_time_t renderTime;
void render() {
    tu_time_t startTime = tu_clock_micros();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

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
    if (debugRendererEnabled)
        renderDebugInfo();
    renderMessages();
    // TODO: Make this a debug flag
    // renderAABB();

    renderTime = tu_clock_micros() - startTime;
}

void drawRect(int x, int y, int width, int height, glm::vec4 color) {
    // GL_CULL_FACE has to be disabled as we are flipping the order of the vertices here, besides we don't really care about it
    glDisable(GL_CULL_FACE);
    glm::mat4 model(1.0f); // Same applies to this VV
    // Make sure to cast these to floats, as mat4<i> is a different type that is not compatible
    glm::mat4 proj = glm::ortho(0.f, (float)viewportWidth, (float)viewportHeight, 0.f, -1.f, 1.f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(width, height, 1.0f));
    generic2dShader->use();
    generic2dShader->set("aColor", color);
    generic2dShader->set("projection", proj);
    generic2dShader->set("model", model);
    rect2DMesh->bind();
    glDrawArrays(GL_TRIANGLES, 0, rect2DMesh->vertexCount);
}

void setViewport(int width, int height) {
    viewportWidth = width, viewportHeight = height;
}

void setDebugRendererEnabled(bool enabled) {
    debugRendererEnabled = enabled;
}