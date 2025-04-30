#pragma once

// Markers for the autogen engine to generate getters, setters, lua, etc.

// Base macros
#ifdef __AUTOGEN__
#define def_inst(...) clang::annotate("OB::def_inst", #__VA_ARGS__)
#define def_prop(...) clang::annotate("OB::def_prop", #__VA_ARGS__)
#define def_prop_category(...) clang::annotate("OB::def_prop_category", #__VA_ARGS__)
#define cframe_position_prop(...) clang::annotate("OB::cframe_position_prop", #__VA_ARGS__)
#define cframe_rotation_prop(...) clang::annotate("OB::cframe_rotation_prop", #__VA_ARGS__)
#else
#define def_inst(...)
#define def_prop(...)
#define def_prop_category(...)
#define cframe_position_prop(...)
#define cframe_rotation_prop(...)
#endif

// Helper macros
#define DEF_INST [[ def_inst() ]]
#define DEF_INST_(...) [[ def_inst(__VA_ARGS__) ]]
#define DEF_INST_ABSTRACT [[ def_inst(abstract) ]]
#define DEF_INST_ABSTRACT_(...) [[ def_inst(__VA_ARGS__, abstract) ]]
#define DEF_INST_SERVICE [[ def_inst(service) ]]
#define DEF_INST_SERVICE_(...) [[ def_inst(__VA_ARGS__, service) ]]
#define DEF_PROP [[ def_prop() ]]
#define DEF_PROP_(...) [[ def_prop(__VA_ARGS__) ]]

// Categories
#define DEF_PROP_CATEGORY(CATEGORY) [[ def_prop_category(category=CATEGORY) ]]

#define AUTOGEN_PREAMBLE \
protected: \
virtual result<PropertyMeta, MemberNotFound> InternalGetPropertyMeta(std::string name) override; \
virtual fallible<MemberNotFound, AssignToReadOnlyMember> InternalSetPropertyValue(std::string name, Data::Variant value) override; \
virtual result<Data::Variant, MemberNotFound> InternalGetPropertyValue(std::string name) override; \
virtual void InternalUpdateProperty(std::string name) override; \
virtual std::vector<std::string> InternalGetProperties() override; \
public: \
const static InstanceType TYPE; \
virtual const InstanceType* GetClass() override; \
private:
