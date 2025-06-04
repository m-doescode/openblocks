#pragma once

// Markers for the autogen engine to generate getters, setters, lua, etc.

// Base macros
#ifdef __AUTOGEN__
#define def_enum(...) clang::annotate("OB::def_enum", #__VA_ARGS__)
#else
#define def_enum(...)
#endif

// Helper macros
#define DEF_ENUM [[ def_enum() ]]