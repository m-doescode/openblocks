#include "codegen.h"
#include "analysis.h"
#include <map>
#include <string>
#include <variant>

using namespace data;

static std::string getLuaMethodFqn(std::string className, std::string methodName) {
    return "__lua_impl_" + className + "__" + methodName;
}

static void writeLuaLibraryGenerator(std::ofstream& out, ClassAnalysis& state) {
    out << "static int lib_gc(lua_State*);"
        << "static int lib_index(lua_State*);"
        << "static int lib_newindex(lua_State*);"
        << "static const struct luaL_Reg metatable [] = {"
        << "    {\"__index\", lib_index},"
        << "    {\"__newindex\", lib_newindex},"
        << "    {NULL, NULL} /* end of array */"
        << "};";

    // Create push function
    out << "void Data::" << state.name << "::PushLuaLibrary(lua_State* L) {\n";
    
    out << "    int n = lua_gettop(L);\n"
        << "    lua_newuserdata(L, 0);\n"
        << "    luaL_newmetatable(L, \"__mt_" << state.name << "\");\n"
        << "    luaL_register(L, NULL, library_metatable);\n"
        << "    lua_setmetatable(L, n+1);\n";

    out << "}\n";


}

void data::writeCodeForClass(std::ofstream& out, std::string headerPath, ClassAnalysis& state) {
    // std::string fqn = "Data::" + state.name;

    // out << "#define __AUTOGEN_EXTRA_INCLUDES__\n";
    // out << "#include \"lua.h\"";
    // out << "#include \"" << headerPath << "\"\n\n";
    // out << "const Data::TypeInfo " << fqn << "::TYPE = {\n"
    //     << "    .name = \"" << fqn << "\",\n"
    //     << "    .deserializer = &" << fqn << "::Deserialize,\n"
    //     << "    .fromLuaValue = %" << fqn << "::FromLuaValue,\n"
    //     << "};\n\n";

    // out << "const Data::TypeInfo& " << fqn << "::GetType() {\n"
    //     << "    return &TYPE;\n"
    //     << "};\n\n";

    // writeLuaLibraryGenerator(out, state);
}