#include "codegen.h"
#include "analysis.h"
#include <fstream>
#include <map>
#include <string>
#include <variant>
#include <vector>

using namespace data;

static std::map<std::string, std::string> MAPPED_TYPE = {
    { "bool", "Data::Bool" },
    { "int", "Data::Int" },
    { "float", "Data::Float" },
    { "std::string", "Data::String" },
    { "glm::vec3", "Vector3" },
};

static std::map<std::string, std::string> LUA_CHECK_FUNCS = {
    { "bool", "lua_toboolean" },
    { "int", "luaL_checkinteger" },
    { "float", "luaL_checknumber" },
    { "std::string", "luaL_checkstring" },
};

static std::string getLuaMethodFqn(std::string className, std::string methodName) {
    return "__lua_impl__" + className + "__" + methodName;
}

static std::string getMtName(std::string type) {
    if (type.starts_with("Data::"))
        return "__mt_" + type.substr(6);
    return "__mt_" + type;
}

static void writeLuaGetArgument(std::ofstream& out, std::string type, int narg, bool member) {
    std::string varname = "arg" + std::to_string(narg);

    std::string checkFunc = LUA_CHECK_FUNCS[type];
    if (checkFunc != "") {
        out << "    " << type << " " << varname << " = " << checkFunc << "(L, " << std::to_string(member ? narg + 1 : narg) << ");\n";
    } else {
        std::string udataname = getMtName(type);
        out << "    " << type << " " << varname << " = *(" << type << "*)luaL_checkudata(L, " << std::to_string(member ? narg + 1 : narg) << ", \"" << udataname << "\");\n";
    }
}

static void writeLuaMethodImpls(std::ofstream& out, ClassAnalysis& state) {
    std::string fqn = "Data::" + state.name;

    // Collect all method names to account for overloaded functions
    std::map<std::string, std::vector<MethodAnalysis>> methods;
    std::map<std::string, std::vector<MethodAnalysis>> staticMethods;

    for (MethodAnalysis method : state.methods) {
        methods[method.name].push_back(method);
    }
    
    for (MethodAnalysis method : state.staticMethods) {
        staticMethods[method.name].push_back(method);
    }

    for (auto& [name, methodImpls] : methods) {
        std::string methodFqn = getLuaMethodFqn(state.name, name);
        out <<  "static int " << methodFqn << "(lua_State* L) {\n"
                "    auto this__ = (Data::Base*)lua_touserdata(L, 1);\n"
                "    if (&this__->GetType() != &" << fqn << "::TYPE) return luaL_typerror(L, 0, \"" << state.name << "\");\n"
                "    " << fqn << "* this_ = (" << fqn << "*)this__;\n\n";
                
        // Currently overloads are not supported

        for (int i = 0; i < methodImpls[0].parameters.size(); i++) {
            writeLuaGetArgument(out, methodImpls[0].parameters[i].type, i, true);
        }

        // Store result
        if (methodImpls[0].returnType != "void")
            out << "    " << methodImpls[0].returnType << " result = ";
        else
            out << "    ";

        // Call function
        out << "this_->" << methodImpls[0].functionName << "(";

        for (int i = 0; i < methodImpls[0].parameters.size(); i++) {
            std::string varname = "arg" + std::to_string(i);
            if (i != 0) out << ", ";
            out << varname;
        }

        out << ");\n";

        // Return result
        if (methodImpls[0].returnType != "void") {
            std::string mappedType = MAPPED_TYPE[methodImpls[0].returnType];
            if (mappedType == "")
                out << "    result.PushLuaValue(L);\n";
            else
                out << "    " << mappedType << "(result).PushLuaValue(L);\n";
        }

        if (methodImpls[0].returnType == "void")
            out << "    return 0;\n";
        else
            out << "    return 1;\n";

        out << "}\n\n";
    }

    for (auto& [name, methodImpls] : staticMethods) {
        std::string methodFqn = getLuaMethodFqn(state.name, name);
        out <<  "static int " << methodFqn << "(lua_State* L) {\n";
                
        // Currently overloads are not supported

        for (int i = 0; i < methodImpls[0].parameters.size(); i++) {
            writeLuaGetArgument(out, methodImpls[0].parameters[i].type, i, false);
        }

        // Store result
        if (methodImpls[0].returnType != "void")
            out << "    " << methodImpls[0].returnType << " result = ";
        else
            out << "    ";

        // Call function
        if (methodImpls[0].functionName == "__ctor")
            out << fqn << "(";
        else
            out << fqn << "::" << methodImpls[0].functionName << "(";

        for (int i = 0; i < methodImpls[0].parameters.size(); i++) {
            std::string varname = "arg" + std::to_string(i);
            if (i != 0) out << ", ";
            out << varname;
        }

        out << ");\n";

        // Return result
        if (methodImpls[0].returnType != "void") {
            std::string mappedType = MAPPED_TYPE[methodImpls[0].returnType];
            if (mappedType == "")
                out << "    result.PushLuaValue(L);\n";
            else
                out << "    " << mappedType << "(result).PushLuaValue(L);\n";
        }

        if (methodImpls[0].returnType == "void")
            out << "    return 0;\n";
        else
            out << "    return 1;\n";

        out << "}\n\n";
    }
}

static void writeLuaValueGenerator(std::ofstream& out, ClassAnalysis& state) {
    std::string fqn = "Data::" + state.name;

    out <<  "static int data_gc(lua_State*);\n"
            "static int data_index(lua_State*);\n"
            "static int data_tostring(lua_State*);\n"
            "static const struct luaL_Reg metatable [] = {\n"
            "    {\"__gc\", data_gc},\n"
            "    {\"__index\", data_index},\n"
            "    {\"__tostring\", data_tostring},\n"
            "    {NULL, NULL} /* end of array */\n"
            "};\n\n";

    out << "void Data::" << state.name << "::PushLuaValue(lua_State* L) const {\n"
           "    int n = lua_gettop(L);\n"

        //    "    // I'm torn... should this be Data::Variant, or Data::Base?\n"
        //    "    // If I ever decouple typing from Data::Base, I'll switch it to variant,\n"
        //    "    // otherwise, it doesn't make much sense to represent it as one\n"
           "    " << fqn << "* userdata = (" << fqn << "*)lua_newuserdata(L, sizeof(" << fqn << "));\n"
           "    new(userdata) " << fqn <<  "(*this);\n"

           "    // Create the library's metatable\n"
           "    luaL_newmetatable(L, \"__mt_" << state.name << "\");\n"
           "    luaL_register(L, NULL, metatable);\n"

           "    lua_setmetatable(L, n+1);\n"
           "}\n\n";
    

    out <<  "result<Data::Variant, LuaCastError> Data::" << state.name << "::FromLuaValue(lua_State* L, int idx) {\n"
            "    " << fqn << "* userdata = (" << fqn << "*) luaL_testudata(L, idx, \"" << getMtName(state.name) << "\");\n"
            "    if (userdata == nullptr)\n"
            "        return LuaCastError(lua_typename(L, idx), \"" << state.name << "\");\n"
            "    return Data::Variant(*userdata);\n"
            "}\n\n";

    // Indexing methods and properties

    out <<  "static int data_index(lua_State* L) {\n"
            "    auto this__ = (Data::Base*)lua_touserdata(L, 1);\n"
            "    if (&this__->GetType() != &" << fqn << "::TYPE) return luaL_typerror(L, 0, \"" << state.name << "\");\n"
            "    " << fqn << "* this_ = (" << fqn << "*)this__;\n"
            "\n"
            "    std::string key(lua_tostring(L, 2));\n"
            "    lua_pop(L, 2);\n"
            "\n";

    out << "    ";

    bool first = true;
    for (PropertyAnalysis prop : state.properties) {
        if (!first) out << " else ";
        first = false;

        out << "if (key == \"" << prop.name << "\") {\n";

        std::string type = MAPPED_TYPE[prop.valueType];
        if (type == "") type = prop.valueType;

        std::string valueExpr;
        if (prop.backingType == PropertyBackingType::Field)
            valueExpr = "this_->" + prop.backingSymbol;
        else if (prop.backingType == PropertyBackingType::Method)
            valueExpr = "this_->" + prop.backingSymbol + "()";

        // This largely depends on the type
        out << "        " << type << "(" << valueExpr << ").PushLuaValue(L);\n";
        out << "        return 1;\n";

        out << "    }";
    }

    std::map<std::string, bool> accountedMethods;
    for (MethodAnalysis method : state.methods) {
        if (accountedMethods[method.name]) continue;
        if (!first) out << " else ";
        first = false;
        accountedMethods[method.name] = true;

        out << "if (key == \"" << method.name << "\") {\n";
        out << "        lua_pushcfunction(L, " << getLuaMethodFqn(state.name, method.name) << ");\n";
        out << "        return 1;\n";

        out << "    }";
    }

    out << "\n\n"
            "    return luaL_error(L, \"%s is not a valid member of %s\\n\", key.c_str(), \"" << state.name << "\");\n"
            "}\n\n";

    // ToString

    out <<  "\nint data_tostring(lua_State* L) {\n"
    "    auto this_ = (" << fqn << "*)lua_touserdata(L, 1);\n"
    "    lua_pushstring(L, std::string(this_->ToString()).c_str());\n"
    "    return 1;\n"
    "}\n\n";

    // Destructor

    out <<  "\nint data_gc(lua_State* L) {\n"
            "    auto this_ = (" << fqn << "*)lua_touserdata(L, 1);\n"
            "    delete this_;\n"
            "    return 0;\n"
            "}\n\n";
}

static void writeLuaLibraryGenerator(std::ofstream& out, ClassAnalysis& state) {
    std::string fqn = "Data::" + state.name;

    out <<  "static int lib_index(lua_State*);\n"
            "static const struct luaL_Reg lib_metatable [] = {\n"
            "    {\"__index\", lib_index},\n"
            "    {NULL, NULL} /* end of array */\n"
            "};\n\n";

    out << "void Data::" << state.name << "::PushLuaLibrary(lua_State* L) {\n"
           "    lua_getglobal(L, \"_G\");\n"
           "    lua_pushstring(L, \"" << state.name << "\");\n"
           "\n"
           "    lua_newuserdata(L, 0);\n"
           "\n"
           "    // Create the library's metatable\n"
           "    luaL_newmetatable(L, \"__mt_lib_" << state.name << "\");\n"
           "    luaL_register(L, NULL, lib_metatable);\n"
           "    lua_setmetatable(L, -2);\n"
           "\n"
           "    lua_rawset(L, -3);\n"
           "    lua_pop(L, 1);\n"
           "}\n\n";
    
    // Indexing methods and properties

    out <<  "static int lib_index(lua_State* L) {\n"
            "    std::string key(lua_tostring(L, 2));\n"
            "    lua_pop(L, 2);\n"
            "\n";

    out << "    ";

    bool first = true;
    for (PropertyAnalysis prop : state.staticProperties) {
        if (!first) out << " else ";
        first = false;

        out << "if (key == \"" << prop.name << "\") {\n";

        std::string type = MAPPED_TYPE[prop.valueType];
        if (type == "") type = prop.valueType;

        std::string valueExpr;
        if (prop.backingType == PropertyBackingType::Field)
            valueExpr = fqn + "::" + prop.backingSymbol;
        else if (prop.backingType == PropertyBackingType::Method)
            valueExpr = fqn + "::" + prop.backingSymbol + "()";

        out << "        " << type << "(" << valueExpr << ").PushLuaValue(L);\n";
        out << "        return 1;\n";

        out << "    }";
    }

    std::map<std::string, bool> accountedMethods;
    for (MethodAnalysis method : state.staticMethods) {
        if (accountedMethods[method.name]) continue;
        if (!first) out << " else ";
        first = false;
        accountedMethods[method.name] = true;

        out << "if (key == \"" << method.name << "\") {\n";
        out << "        lua_pushcfunction(L, " << getLuaMethodFqn(state.name, method.name) << ");\n";
        out << "        return 1;\n";

        out << "    }";
    }

    out << "\n\n"
            "    return luaL_error(L, \"%s is not a valid member of %s\\n\", key.c_str(), \"" << state.name << "\");\n"
            "}\n\n";
}

void data::writeCodeForClass(std::ofstream& out, std::string headerPath, ClassAnalysis& state) {
    std::string fqn = "Data::" + state.name;

    out << "#define __AUTOGEN_EXTRA_INCLUDES__\n";
    out << "#include \"" << headerPath << "\"\n\n";
    out << "#include \"datatypes/meta.h\"\n";
    out << "#include \"lua.h\"\n\n";
    out << "const Data::TypeInfo " << fqn << "::TYPE = {\n"
        << "    .name = \"" << state.serializedName << "\",\n"
        << "    .deserializer = &" << fqn << "::Deserialize,\n";
    if (state.hasFromString) out << "    .fromString = &" << fqn << "::FromString,\n";
    out << "    .fromLuaValue = &" << fqn << "::FromLuaValue,\n"
        << "};\n\n";

    out << "const Data::TypeInfo& " << fqn << "::GetType() const {\n"
        << "    return TYPE;\n"
        << "};\n\n";

    writeLuaMethodImpls(out, state);
    writeLuaValueGenerator(out, state);
    writeLuaLibraryGenerator(out, state);
}