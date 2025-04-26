#pragma once

// Markers for the autogen engine to generate getters, setters, lua, etc.

#ifdef __AUTOGEN__
#define def_inst(...) clang::annotate("OB::def_inst", #__VA_ARGS__)

#define def_prop(...) clang::annotate("OB::def_prop", #__VA_ARGS__)

#define cframe_position_prop(...) clang::annotate("OB::cframe_position_prop", #__VA_ARGS__)
#define cframe_rotation_prop(...) clang::annotate("OB::cframe_rotation_prop", #__VA_ARGS__)
#else
#define def_inst(...)
#define def_prop(...)
#define cframe_position_prop(...)
#define cframe_rotation_prop(...)
#endif

#define AUTOGEN_PREAMBLE \
private: \
result<PropertyMeta, MemberNotFound> InternalGetPropertyMeta(std::string name) override; \
fallible<MemberNotFound, AssignToReadOnlyMember> InternalSetPropertyValue(std::string name, Data::Variant value) override; \
result<Data::Variant, MemberNotFound> InternalGetPropertyValue(std::string name) override; \
private:
