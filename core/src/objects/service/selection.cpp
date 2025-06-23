#include "selection.h"
#include "objects/datamodel.h"
#include <algorithm>
#include <memory>

Selection::Selection(): Service(&TYPE) {
}

Selection::~Selection() = default;

void Selection::InitService() {
    if (initialized) return;
    initialized = true;
}

std::vector<std::shared_ptr<Instance>> Selection::Get() {
    return selection;
}

void Selection::Set(std::vector<std::shared_ptr<Instance>> newSelection) {
    selection = newSelection;
    SelectionChanged->Fire();
}

void Selection::Add(std::vector<std::shared_ptr<Instance>> instances) {
    for (auto inst : instances) {
        if (std::find(selection.begin(), selection.end(), inst) == selection.end()) {
            selection.push_back(inst);
        }
    }
    SelectionChanged->Fire();
}

void Selection::Remove(std::vector<std::shared_ptr<Instance>> instances) {
    for (auto inst : instances) {
        std::vector<std::shared_ptr<Instance>>::iterator p;
        if ((p = std::find(selection.begin(), selection.end(), inst)) != selection.end()) {
            selection.erase(p);
        }
    }
    SelectionChanged->Fire();
}