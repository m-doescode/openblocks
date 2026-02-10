#pragma once

#include "type.h" // IWYU pragma: keep

#define INSTANCE_HEADER_SOURCE  \
public: \
    static inline const InstanceType2& Type() { \
        static const InstanceType2 type = __buildType(); \
        return type; \
    } \
\
    inline const InstanceType2& GetType() override { \
        return Type();\
    } \
private:
