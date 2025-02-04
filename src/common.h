#pragma once
#include "objects/base/instance.h"
#include "objects/workspace.h"
#include "camera.h"
#include <functional>
#include <memory>

class Instance;
// typedef std::function<void(std::shared_ptr<Instance> element, std::optional<std::shared_ptr<Instance>> newParent)> HierarchyUpdateHandler;
typedef std::function<void(InstanceRef object, std::optional<InstanceRef> oldParent, std::optional<InstanceRef> newParent)> HierarchyPreUpdateHandler;
typedef std::function<void(InstanceRef object, std::optional<InstanceRef> oldParent, std::optional<InstanceRef> newParent)> HierarchyPostUpdateHandler;
typedef std::function<void(std::vector<InstanceRefWeak> oldSelection, std::vector<InstanceRefWeak> newSelection, bool fromExplorer)> SelectionUpdateHandler;

// TEMPORARY COMMON DATA FOR VARIOUS INTERNAL COMPONENTS

extern Camera camera;
extern std::shared_ptr<DataModel> dataModel;
extern std::shared_ptr<Workspace> workspace;
extern std::optional<HierarchyPreUpdateHandler> hierarchyPreUpdateHandler;
extern std::optional<HierarchyPostUpdateHandler> hierarchyPostUpdateHandler;

void setSelection(std::vector<InstanceRefWeak> newSelection, bool fromExplorer = false);
const std::vector<InstanceRefWeak> getSelection();
void addSelectionListener(SelectionUpdateHandler handler);