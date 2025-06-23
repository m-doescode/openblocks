#pragma once

#include "datatypes/signal.h"
#include "datatypes/variant.h"
#include "objects/base/instance.h"
#include "utils.h"
#include <deque>
#include <memory>
#include <stack>

struct UndoStatePropertyChanged {
    std::shared_ptr<Instance> affectedInstance;
    std::string property;
    Variant oldValue;
    Variant newValue;
};

struct UndoStateInstanceCreated {
    std::shared_ptr<Instance> instance;
    std::shared_ptr<Instance> newParent;
};

struct UndoStateInstanceRemoved {
    std::shared_ptr<Instance> instance;
    std::shared_ptr<Instance> oldParent;
};

struct UndoStateInstanceReparented {
    std::shared_ptr<Instance> instance;
    nullable std::shared_ptr<Instance> oldParent;
    nullable std::shared_ptr<Instance> newParent;
};

struct UndoStateSelectionChanged {
    std::vector<std::shared_ptr<Instance>> oldSelection;
    std::vector<std::shared_ptr<Instance>> newSelection;
};

typedef std::variant<UndoStatePropertyChanged, UndoStateInstanceCreated, UndoStateInstanceRemoved, UndoStateInstanceReparented, UndoStateSelectionChanged> UndoStateChange;
typedef std::vector<UndoStateChange> UndoState;
typedef std::function<void(bool canUndo, bool canRedo)> UndoStateChangedListener;

class UndoHistory  {
    // Ignore PushState requests
    bool processingUndo = false;
    std::deque<UndoState> undoHistory;
    std::stack<UndoState> redoHistory;

    UndoStateChangedListener undoStateListener;
public:
    int maxBufferSize = 100;

    void PushState(UndoState);
    void Undo();
    void Redo();

    void SetUndoStateListener(UndoStateChangedListener listener);
};