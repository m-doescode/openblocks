// All meta code for the project
// This is stored in one file because different parts here
// depend on other parts, and initialization order between
// translation units is not guaranteed

#include "objects/meta.h"
#include "datatypes/meta.h"
#include "datatypes/enum.h"
#include "datatypes/primitives.h"
#include "enum/part.h"
#include "enum/surface.h"
#include <typeindex>
#include <vector>

#define DECLINST(className) class className { public: static const InstanceType& Type(); };
#define DECLTYPE(className) class className { public: static const TypeDesc TYPE; }

// Enums

template <typename T> inline std::type_index index() {
    return std::type_index(typeid(T));
}

// TODO: Move this into its own file
std::map<std::type_index, const Enum*> ENUM_TYPE = {
    { index<SurfaceType>(), &EnumType::SurfaceType },
    { index<NormalId>(), &EnumType::NormalId },
    { index<PartType>(), &EnumType::PartType },
};

// Integral data types

// DECLTYPE(Vector3);
// DECLTYPE(CFrame);
// DECLTYPE(Color3);
// DECLTYPE(InstanceRef);
// DECLTYPE(SignalRef);
// DECLTYPE(SignalConnectionRef);

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

std::map<std::string, const TypeDesc*> TYPE_BY_NAME = {
    { "null", &NULL_TYPE },
    { "bool", &BOOL_TYPE },
    { "int", &INT_TYPE },
    { "float", &FLOAT_TYPE },
    { "string", &STRING_TYPE },
    { "Vector3", &Vector3::TYPE },
    { "CoordinateFrame", &CFrame::TYPE },
    { "Color3", &Color3::TYPE },
    { "Ref", &InstanceRef::TYPE },
    { "token", &EnumItem::TYPE },
};

// Instances

DECLINST(DataModel);
DECLINST(BasePart);
DECLINST(Part);
DECLINST(WedgePart);
DECLINST(Snap);
DECLINST(Weld);
DECLINST(Rotate);
DECLINST(RotateV);
DECLINST(Motor6D);
DECLINST(JointInstance);
DECLINST(Script);
DECLINST(Model);
DECLINST(Message);
DECLINST(Hint);
DECLINST(ClickDetector);
DECLINST(Camera);
// DECLINST(Folder);
DECLINST(Workspace);
DECLINST(JointsService);
DECLINST(ScriptContext);
DECLINST(ServerScriptService);
DECLINST(Selection);

std::map<std::string, const InstanceType*> INSTANCE_MAP = {
    { "Instance", &Instance::Type() },
    { "DataModel", &DataModel::Type() },

    { "BasePart", &BasePart::Type() },
    { "Part", &Part::Type() },
    { "WedgePart", &WedgePart::Type() },
    { "Snap", &Snap::Type() },
    { "Weld", &Weld::Type() },
    { "Rotate", &Rotate::Type() },
    { "RotateV", &RotateV::Type() },
    { "Motor6D", &Motor6D::Type() },
    { "JointInstance", &JointInstance::Type() },
    { "Script", &Script::Type() },
    { "Model", &Model::Type() },
    { "Message", &Message::Type() },
    { "Hint", &Hint::Type() },
    { "ClickDetector", &ClickDetector::Type() },
    { "Camera", &Camera::Type() },
    // { "Folder", &Folder::Type() },

    // Services

    { "Workspace", &Workspace::Type() },
    { "JointsService", &JointsService::Type() },
    { "ScriptContext", &ScriptContext::Type() },
    { "ServerScriptService", &ServerScriptService::Type() },
    { "Selection", &Selection::Type() },
};