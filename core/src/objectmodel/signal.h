#pragma once

#include <memory>
#include <functional>
#include <type_traits>

class SignalSource;
class Instance;

struct InstanceSignal {
    std::string name;
    std::function<SignalSource(std::shared_ptr<Instance>)> getter;
};

template <typename C>
std::enable_if_t<std::is_base_of_v<Instance, C>, InstanceSignal> def_signal(std::string name, SignalSource C::* signal) {
    return {
        name,
        [signal](std::shared_ptr<Instance> instance) {
            auto obj = std::dynamic_pointer_cast<C>(instance);
            return obj.get()->*signal;
        }
    };
}