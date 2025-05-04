#pragma once

// Markers for the autogen engine to generate getters, setters, lua, etc.

// Base macros
#ifdef __AUTOGEN__
#define def_data(...) clang::annotate("OB::def_data", #__VA_ARGS__)
#define def_data_prop(...) clang::annotate("OB::def_data_prop", #__VA_ARGS__)
#define def_data_method(...) clang::annotate("OB::def_data_method", #__VA_ARGS__)
#define def_data_ctor(...) clang::annotate("OB::def_data_ctor", #__VA_ARGS__)
#else
#define def_data(...)
#define def_data_prop(...)
#define def_data_method(...)
#define def_data_ctor(...)
#define def_data_op(...)
#endif

// Helper macros
#define DEF_DATA [[ def_data() ]] // Data declaration
#define DEF_DATA_(...) [[ def_data(__VA_ARGS__) ]]
#define DEF_DATA_PROP [[ def_data_prop() ]] // Property. Getter or field
#define DEF_DATA_METHOD [[ def_data_method() ]] // Method
#define DEF_DATA_CTOR [[ def_data_ctor() ]] // Constructor (i.e. .new)
#define DEF_DATA_OP [[ def_data_op() ]] // Operator (e.g. +, -, *)

#define AUTOGEN_PREAMBLE_DATA \
public: \
virtual const TypeInfo& GetType() const override; \
static const TypeInfo TYPE; \
virtual void PushLuaValue(lua_State*) const override; \
static result<Data::Variant, LuaCastError> FromLuaValue(lua_State*, int idx); \
private:
