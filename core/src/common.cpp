// TEMPORARY COMMON DATA FOR DIFFERENT INTERNAL COMPONENTS

#include "objects/datamodel.h"
#include "datatypes/meta.h"
#include "common.h"
#include <memory>

Camera camera(glm::vec3(0.0, 0.0, 3.0));
//std::vector<Part> parts;
std::shared_ptr<DataModel> gDataModel = DataModel::New();
std::shared_ptr<DataModel> editModeDataModel = gDataModel;
std::optional<HierarchyPreUpdateHandler> hierarchyPreUpdateHandler;
std::optional<HierarchyPostUpdateHandler> hierarchyPostUpdateHandler;
std::shared_ptr<Handles> editorToolHandles = Handles::New();


std::vector<InstanceRefWeak> currentSelection;
std::vector<SelectionUpdateHandler> selectionUpdateListeners;
std::vector<PropertyUpdateHandler> propertyUpdatelisteners;

void setSelection(std::vector<InstanceRefWeak> newSelection, bool fromExplorer) {
    for (SelectionUpdateHandler handler : selectionUpdateListeners) {
        handler(currentSelection, newSelection, fromExplorer);
    }

    currentSelection = newSelection;
}

const std::vector<InstanceRefWeak> getSelection() {
    return currentSelection;
}

void addSelectionListener(SelectionUpdateHandler handler) {
    selectionUpdateListeners.push_back(handler);
}

void sendPropertyUpdatedSignal(InstanceRef instance, std::string property, Data::Variant newValue) {
    for (PropertyUpdateHandler handler : propertyUpdatelisteners) {
        handler(instance, property, newValue);
    }
}

void addPropertyUpdateListener(PropertyUpdateHandler handler) {
    propertyUpdatelisteners.push_back(handler);
}