#pragma once
#include "objects/workspace.h"
#include "camera.h"
#include <functional>
#include <memory>

class Instance;
// typedef std::function<void(std::shared_ptr<Instance> element, std::optional<std::shared_ptr<Instance>> newParent)> HierarchyUpdateHandler;
typedef std::function<void(InstanceRef object, std::optional<InstanceRef> oldParent, std::optional<InstanceRef> newParent)> HierarchyPreUpdateHandler;
typedef std::function<void(InstanceRef object, std::optional<InstanceRef> oldParent, std::optional<InstanceRef> newParent)> HierarchyPostUpdateHandler;

// TEMPORARY COMMON DATA FOR DIFFERENT INTERNAL COMPONENTS

extern Camera camera;
extern std::shared_ptr<Workspace> workspace;
extern std::optional<HierarchyPreUpdateHandler> hierarchyPreUpdateHandler;
extern std::optional<HierarchyPostUpdateHandler> hierarchyPostUpdateHandler;