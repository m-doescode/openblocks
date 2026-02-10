#pragma once

#include "type.h" // IWYU pragma: keep

// Use when the file implementing __buildType is available
#define INSTANCE_HEADER \
private: \
    static InstanceType __buildType(); \
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
