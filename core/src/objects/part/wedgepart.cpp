#include "wedgepart.h"
// #include <reactphysics3d/collision/ConvexMesh.h>
// #include <reactphysics3d/collision/shapes/ConvexMeshShape.h>

// rp::ConvexMesh* wedgePhysMesh;

WedgePart::WedgePart(): BasePart(&TYPE) {
}

WedgePart::WedgePart(PartConstructParams params): BasePart(&TYPE, params) {                      
    
}

// void WedgePart::updateCollider(rp::PhysicsCommon* common) {
//     Logger::fatalError("Wedges are currently disabled! Please do not use them or your editor may crash\n");
//     rp::ConvexMeshShape* shape = common->createConvexMeshShape(wedgePhysMesh, glmToRp(size * glm::vec3(0.5f)));

//     // Recreate the rigidbody if the shape changes
//     if (rigidBody->getNbColliders() > 0
//         && dynamic_cast<rp::ConvexMeshShape*>(rigidBody->getCollider(0)->getCollisionShape())->getScale() != shape->getScale()) {
//         // TODO: This causes Touched to get called twice. Fix this.
//         rigidBody->removeCollider(rigidBody->getCollider(0));
//         rigidBody->addCollider(shape, rp::Transform());
//     }

//     if (rigidBody->getNbColliders() == 0)
//         rigidBody->addCollider(shape, rp::Transform());
// }

// void WedgePart::createWedgeShape(rp::PhysicsCommon* common) {
//     // https://www.reactphysics3d.com/documentation/index.html#creatingbody
//     float vertices[] = {
//         //     X   Y   Z
//         /*0*/ -1,  1,  1, // 0
//         /*1*/ -1, -1,  1, // |
//         /*2*/ -1, -1, -1, // 1---2

//         /*3*/  1,  1,  1,
//         /*4*/  1, -1,  1,
//         /*5*/  1, -1, -1,
//     };

//     //     -x        +x
//     //  +z 1----------4
//     //     |  bottom  |
//     //  -z 2----------5

//     //     -x        +x
//     //  +y 0----------3
//     //     |  front   |
//     //  -y 1----------4

//     //      -x        +x
//     //  +yz 0----------3
//     //      |  slope   |
//     //  -yz 2----------5

//     int indices[] = {
//         // Base
//         1, 2, 5, 4,

//         // Back-face
//         0, 1, 4, 3,
//         // 4, 1, 0, 3,

//         // Slope
//         0, 2, 5, 3,
//         // 3, 5, 2, 0,

//         // Sides
//         0, 1, 2,
//         3, 4, 5,
//     };
    
//     // Description of the six faces of the convex mesh
//     rp::PolygonVertexArray::PolygonFace* polygonFaces = new rp::PolygonVertexArray::PolygonFace[5];
//     polygonFaces[0] = { 4, 0 }; // Bottom
//     polygonFaces[1] = { 4, 4 }; // Front
//     polygonFaces[2] = { 4, 8 }; // Slope
//     polygonFaces[3] = { 3, 12 }; // Side
//     polygonFaces[4] = { 3, 15 }; // Side
    
//     // Create the polygon vertex array
//     rp::PolygonVertexArray polygonVertexArray(6, vertices, 3 * sizeof(float), indices, sizeof(int), 5, polygonFaces,
//     rp::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
//     rp::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE);
    
//     // Create the convex mesh
//     std::vector<rp::Message> messages;
//     // wedgePhysMesh = common->createConvexMesh(polygonVertexArray, messages);
// }