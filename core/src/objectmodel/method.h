#pragma once

#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "datatypes/variant.h"
#include "error/error.h"
#include "objectmodel/datatypes.h"

using MethodFlags = int;

using GenericResult = std::variant<Variant, std::shared_ptr<Error>>;
using GenericMethod = std::function<GenericResult(std::shared_ptr<Instance> obj, std::vector<Variant> args)>;

struct InstanceMethod {
    std::string name;
    MethodFlags flags;
    GenericMethod method;
    TypeMeta returnType;
    std::vector<TypeMeta> paramTypes;
};

template <typename T, typename C, typename... Args>
InstanceMethod def_method(std::string name, T (C::*func)(Args...)) {
    return {
        name,
        0,
        [func](std::shared_ptr<Instance> instance, std::vector<Variant> args) -> GenericResult {
            auto obj = std::dynamic_pointer_cast<C>(instance);

            if (args.size() < sizeof...(Args))
                return std::monostate(); // TODO: Throw error

            int i = 0;
            std::tuple<C*, Args...> targs = { obj.get(), args.at(i++).get<Args>()... };

            return GenericResult(std::apply(func, targs));
        },
        type_meta_of<T>(),
        { type_meta_of<Args>()... }
    };
}