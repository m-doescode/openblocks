#pragma once

// Markers for the autogen engine to generate getters, setters, lua, etc.

#define def_inst(...) clang::annotate("OB::def_inst", #__VA_ARGS__)

#define def_prop(...) clang::annotate("OB::def_prop", #__VA_ARGS__)

#define cframe_position_prop(...) clang::annotate("OB::cframe_position_prop", #__VA_ARGS__)
#define cframe_rotation_prop(...) clang::annotate("OB::cframe_rotation_prop", #__VA_ARGS__)