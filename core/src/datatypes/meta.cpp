#include "meta.h"
#include "datatypes/enum.h"
#include "datatypes/primitives.h"
#include "enum/part.h"
#include "enum/surface.h"
#include <typeindex>
#include <vector>

#define DECLTYPE(className) class className { public: static const TypeDesc TYPE; }

template <typename T> inline std::type_index index() {
    return std::type_index(typeid(T));
}

// TODO: Move this into its own file
std::map<std::type_index, const Enum*> ENUM_TYPE = {
    { index<SurfaceType>(), &EnumType::SurfaceType },
    { index<NormalId>(), &EnumType::NormalId },
    { index<PartType>(), &EnumType::PartType },
};

DECLTYPE(Vector3);
DECLTYPE(CFrame);
DECLTYPE(Color3);
DECLTYPE(InstanceRef);
DECLTYPE(SignalRef);
DECLTYPE(SignalConnectionRef);

std::map<std::type_index, TypeMeta> INTEGRAL_TYPE_META = {
    { index<std::monostate>(), &NULL_TYPE },
    { index<void>(), &NULL_TYPE },
    { index<bool>(), &BOOL_TYPE },
    { index<int>(), &INT_TYPE },
    { index<float>(), &FLOAT_TYPE },
    { index<std::string>(), &STRING_TYPE },
    { index<std::vector<Variant>>(), &ARRAY_TYPE },

    // Complex types
    { index<Vector3>(), &Vector3::TYPE },
    { index<CFrame>(), &CFrame::TYPE },
    { index<Color3>(), &Color3::TYPE },
    { index<InstanceRef>(), &InstanceRef::TYPE },
    { index<SignalRef>(), &SignalRef::TYPE },
    { index<SignalConnectionRef>(), &SignalConnectionRef::TYPE },
    { index<Enum>(), &Enum::TYPE },
    { index<EnumItem>(), &EnumItem::TYPE },
};