#pragma once

#include <string>

struct InstanceType2 {
    std::string className;
};

template <typename T /* TODO: Add SFINAE */ >
const InstanceType2 make_instance_type(std::string name) {
    return { name };
}

#define INSTANCE_HEADER_SOURCE  \
public: \
    static inline const InstanceType2& Type() { \
        static const InstanceType2 type = __buildType(); \
        return type; \
    } \
private: