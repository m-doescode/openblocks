#pragma once
#include "objects/base/instance.h"
#include "handles.h"
#include "objects/service/workspace.h"
#include "objects/datamodel.h"
#include "camera.h"
#include <functional>
#include <memory>

class Instance;
// typedef std::function<void(std::shared_ptr<Instance> element, std::optional<std::shared_ptr<Instance>> newParent)> HierarchyUpdateHandler;
typedef std::function<void(std::shared_ptr<Instance> object, std::optional<std::shared_ptr<Instance>> oldParent, std::optional<std::shared_ptr<Instance>> newParent)> HierarchyPreUpdateHandler;
typedef std::function<void(std::shared_ptr<Instance> object, std::optional<std::shared_ptr<Instance>> oldParent, std::optional<std::shared_ptr<Instance>> newParent)> HierarchyPostUpdateHandler;
typedef std::function<void(std::shared_ptr<Instance> instance, std::string property, Variant newValue)> PropertyUpdateHandler;

// TEMPORARY COMMON DATA FOR VARIOUS INTERNAL COMPONENTS

extern Camera camera;
extern std::shared_ptr<DataModel> gDataModel;
extern std::shared_ptr<DataModel> editModeDataModel;
inline std::shared_ptr<Workspace> gWorkspace() { return std::dynamic_pointer_cast<Workspace>(gDataModel->services["Workspace"]); }
extern std::optional<HierarchyPreUpdateHandler> hierarchyPreUpdateHandler;
extern std::optional<HierarchyPostUpdateHandler> hierarchyPostUpdateHandler;
extern Handles editorToolHandles;

void sendPropertyUpdatedSignal(std::shared_ptr<Instance> instance, std::string property, Variant newValue);
void addPropertyUpdateListener(PropertyUpdateHandler handler);