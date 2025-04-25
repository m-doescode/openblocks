#pragma once

// Markers for the autogen engine to generate getters, setters, lua, etc.

#define INSTANCE [[clang::annotate("OB::INSTANCE")]]

#define def_prop(...) clang::annotate("OB::def_prop", #__VA_ARGS__)