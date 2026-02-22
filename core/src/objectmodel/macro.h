#pragma once

#include "type.h" // IWYU pragma: keep

// Use when the file implementing __buildType is available
#define INSTANCE_HEADER \
private: \
    static InstanceType __buildType(); \
public: \
    static const InstanceType& Type(); \
\
    inline const InstanceType& GetType() override { \
        return Type();\
    } \
private:

#define INSTANCE_IMPL(_ClassName) \
const InstanceType& _ClassName::Type() { \
    static const InstanceType type = __buildType(); \
    return type; \
}

// Use when __buildType is implemented inline, or when the implementation
// of the Instance is not available/readable
#define INSTANCE_HEADER_SOURCE \
public: \
    static inline const InstanceType& Type() { \
        static const InstanceType type = __buildType(); \
        return type; \
    } \
\
    inline const InstanceType& GetType() override { \
        return Type();\
    } \
private:
