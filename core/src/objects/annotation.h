#pragma once

// Markers for the autogen engine to generate getters, setters, lua, etc.

#ifdef __AUTOGEN__
#define def_inst(...) clang::annotate("OB::def_inst", #__VA_ARGS__)
#define INSTANCE [[ def_inst() ]]
#define INSTANCE_WITH(...) [[ def_inst(__VA_ARGS__) ]]
#define INSTANCE_SERVICE(...) [[ def_inst(__VA_ARGS__, service) ]]

#define def_prop(...) clang::annotate("OB::def_prop", #__VA_ARGS__)

#define cframe_position_prop(...) clang::annotate("OB::cframe_position_prop", #__VA_ARGS__)
#define cframe_rotation_prop(...) clang::annotate("OB::cframe_rotation_prop", #__VA_ARGS__)
#else
#define def_inst(...)
#define INSTANCE
#define INSTANCE_WITH(...)
#define INSTANCE_SERVICE(...)
#define def_prop(...)
#define cframe_position_prop(...)
#define cframe_rotation_prop(...)
#endif

#define AUTOGEN_PREAMBLE \
protected: \
result<PropertyMeta, MemberNotFound> InternalGetPropertyMeta(std::string name) override; \
fallible<MemberNotFound, AssignToReadOnlyMember> InternalSetPropertyValue(std::string name, Data::Variant value) override; \
result<Data::Variant, MemberNotFound> InternalGetPropertyValue(std::string name) override; \
std::vector<std::string> InternalGetProperties() override; \
public: \
const static InstanceType TYPE; \
virtual const InstanceType* GetClass() override; \
private:
