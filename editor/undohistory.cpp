#include "undohistory.h"
#include "common.h"
#include "objects/service/selection.h"

void UndoHistory::PushState(UndoState state) {
    if (processingUndo) return; // Ignore PushState requests when changes are initiated by us
    redoHistory = {};

    if (maxBufferSize != -1 && (int)undoHistory.size() > maxBufferSize)
        undoHistory.pop_front();

    undoHistory.push_back(state);
    undoStateListener();
}

void UndoHistory::Undo() {
    if (undoHistory.size() == 0) return;
    UndoState state = undoHistory.back();
    undoHistory.pop_back();
    redoHistory.push(state);

    processingUndo = true;

    for (UndoStateChange& change : state) {
        // https://stackoverflow.com/a/63483353
        if (auto v = std::get_if<UndoStatePropertyChanged>(&change)) {
            // The old value used to be valid, so it still should be...
            v->affectedInstance->SetProperty(v->property, v->oldValue).expect();
        } else if (auto v = std::get_if<UndoStateInstanceCreated>(&change)) {
            v->instance->SetParent(nullptr);
        } else if (auto v = std::get_if<UndoStateInstanceRemoved>(&change)) {
            v->instance->SetParent(v->oldParent);
        } else if (auto v = std::get_if<UndoStateInstanceReparented>(&change)) {
            v->instance->SetParent(v->oldParent);
        } else if (auto v = std::get_if<UndoStateSelectionChanged>(&change)) {
            gDataModel->GetService<Selection>()->Set(v->oldSelection);
        }
    }

    processingUndo = false;
    undoStateListener();
}

void UndoHistory::Redo() {
    if (redoHistory.size() == 0) return;
    UndoState state = redoHistory.top();
    redoHistory.pop();
    undoHistory.push_back(state);

    processingUndo = true;

    for (UndoStateChange& change : state) {
        // https://stackoverflow.com/a/63483353
        if (auto v = std::get_if<UndoStatePropertyChanged>(&change)) {
            // The old value used to be valid, so it still should be...
            v->affectedInstance->SetProperty(v->property, v->newValue).expect();
        } else if (auto v = std::get_if<UndoStateInstanceCreated>(&change)) {
            v->instance->SetParent(v->newParent);
        } else if (auto v = std::get_if<UndoStateInstanceRemoved>(&change)) {
            v->instance->SetParent(nullptr);
        } else if (auto v = std::get_if<UndoStateInstanceReparented>(&change)) {
            v->instance->SetParent(v->newParent);
        } else if (auto v = std::get_if<UndoStateSelectionChanged>(&change)) {
            gDataModel->GetService<Selection>()->Set(v->newSelection);
        }
    }

    processingUndo = false;
    undoStateListener();
}

void UndoHistory::Reset() {
    processingUndo = false;
    undoHistory.clear();
    redoHistory = {};
    undoStateListener();
}

void UndoHistory::SetUndoStateListener(UndoStateChangedListener listener) {
    undoStateListener = listener;
}
