#pragma once

// Helper struct used for remapping reference when cloning/serializing

#include <map>
#include <memory>
#include <vector>
class Instance;

template <typename T>
struct __RefState {
    std::map<std::shared_ptr<Instance>, std::shared_ptr<Instance>> remappedInstances;
    std::map<std::shared_ptr<Instance>, std::vector<T>> refsAwaitingRemap;
};

template <typename T>
using RefState = std::shared_ptr<__RefState<T>>;