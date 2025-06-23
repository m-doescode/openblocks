// TEMPORARY COMMON DATA FOR DIFFERENT INTERNAL COMPONENTS

#include "objects/datamodel.h"
#include "datatypes/variant.h"
#include "common.h"
#include <memory>

Camera camera(glm::vec3(0.0, 0.0, 3.0));
//std::vector<Part> parts;
std::shared_ptr<DataModel> gDataModel = DataModel::New();
std::shared_ptr<DataModel> editModeDataModel = gDataModel;
std::optional<HierarchyPreUpdateHandler> hierarchyPreUpdateHandler;
std::optional<HierarchyPostUpdateHandler> hierarchyPostUpdateHandler;
Handles editorToolHandles;


std::vector<std::shared_ptr<Instance>> currentSelection;
std::vector<PropertyUpdateHandler> propertyUpdatelisteners;


void sendPropertyUpdatedSignal(std::shared_ptr<Instance> instance, std::string property, Variant newValue) {
    for (PropertyUpdateHandler handler : propertyUpdatelisteners) {
        handler(instance, property, newValue);
    }
}

void addPropertyUpdateListener(PropertyUpdateHandler handler) {
    propertyUpdatelisteners.push_back(handler);
}