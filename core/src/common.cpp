// TEMPORARY COMMON DATA FOR DIFFERENT INTERNAL COMPONENTS

#include "common.h"
#include <memory>

Camera camera(glm::vec3(0.0, 0.0, 3.0));
//std::vector<Part> parts;
std::shared_ptr<DataModel> dataModel = DataModel::New();
std::optional<HierarchyPreUpdateHandler> hierarchyPreUpdateHandler;
std::optional<HierarchyPostUpdateHandler> hierarchyPostUpdateHandler;
std::shared_ptr<Handles> editorToolHandles = Handles::New();


std::vector<InstanceRefWeak> currentSelection;
std::vector<SelectionUpdateHandler> selectionUpdateHandlers;

void setSelection(std::vector<InstanceRefWeak> newSelection, bool fromExplorer) {
    for (SelectionUpdateHandler handler : selectionUpdateHandlers) {
        handler(currentSelection, newSelection, fromExplorer);
    }

    currentSelection = newSelection;
}

const std::vector<InstanceRefWeak> getSelection() {
    return currentSelection;
}

void addSelectionListener(SelectionUpdateHandler handler) {
    selectionUpdateHandlers.push_back(handler);
}