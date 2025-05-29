#pragma once

// Helper struct used for remapping reference when cloning/serializing

#include "datatypes/base.h"
#include <map>
#include <memory>
#include <vector>
class Instance;

template <typename T, typename U, typename K>
struct __RefState {
    std::map<K, U> remappedInstances;
    std::map<K, std::vector<T>> refsAwaitingRemap;
    int count = 0;
};

template <typename T, typename U, typename K>
using RefState = std::shared_ptr<__RefState<T, U, K>>;

typedef __RefState<std::pair<std::shared_ptr<Instance>, std::string>, std::shared_ptr<Instance>, std::shared_ptr<Instance>> __RefStateClone;
typedef __RefState<pugi::xml_node, std::string, std::shared_ptr<Instance>> __RefStateSerialize;
typedef __RefState<std::pair<std::shared_ptr<Instance>, std::string>, std::shared_ptr<Instance>, std::string> __RefStateDeserialize;

typedef std::shared_ptr<__RefStateClone> RefStateClone;
typedef std::shared_ptr<__RefStateSerialize> RefStateSerialize;
typedef std::shared_ptr<__RefStateDeserialize> RefStateDeserialize;