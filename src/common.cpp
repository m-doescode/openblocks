// TEMPORARY COMMON DATA FOR DIFFERENT INTERNAL COMPONENTS

#include "common.h"
#include <memory>

Camera camera(glm::vec3(0.0, 0.0, 3.0));
//std::vector<Part> parts;
std::shared_ptr<Workspace> workspace = Workspace::New();
std::optional<HierarchyPreUpdateHandler> hierarchyPreUpdateHandler;
std::optional<HierarchyPostUpdateHandler> hierarchyPostUpdateHandler;