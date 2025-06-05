#include "codegen.h"
#include "analysis.h"
#include <fstream>
#include <map>
#include <string>
#include <variant>
#include <vector>

using namespace data;

static std::map<std::string, std::string> MAPPED_TYPE = {
};

static std::map<std::string, std::string> LUA_CHECK_FUNCS = {
    { "bool", "lua_toboolean" },
    { "int", "luaL_checkinteger" },
    { "float", "luaL_checknumber" },
    { "std::string", "luaL_checkstring" },
};

static std::map<std::string, std::string> LUA_TEST_FUNCS = {
    { "bool", "lua_isboolean" },
    { "int", "lua_isinteger" },
    { "float", "lua_isnumber" },
    { "std::string", "lua_isstring" },
};

static std::map<std::string, std::string> LUA_PUSH_FUNCS = {
    { "bool", "lua_pushboolean" },
    { "int", "lua_pushinteger" },
    { "float", "lua_pushnumber" },
    // Handled specially
    // { "std::string", "lua_pushstring" },
};

static std::string getLuaMethodFqn(std::string className, std::string methodName) {
    return "__lua_impl__" + className + "__" + methodName;
}

static std::string getMtName(std::string type) {
    // if (type.starts_with("Data::"))
    //     return "__mt_" + type.substr(6);
    return "__mt_" + type;
}

static std::string pushLuaValue(std::string type, std::string expr) {
    if (type == "std::string")
        return "lua_pushstring(L, " + expr + ".c_str())";
    std::string mappedType = MAPPED_TYPE[type];
    if (mappedType != "")
        return mappedType + "(" + expr + ").PushLuaValue(L)";
    std::string pushFunc = LUA_PUSH_FUNCS[type];
    if (pushFunc != "")
        return pushFunc + "(L, " + expr + ")";
    return expr + ".PushLuaValue(L)";
}

static void writeLuaGetArgument(std::ofstream& out, std::string type, int narg, bool member) {
    std::string varname = "arg" + std::to_string(narg);
    narg += 1; // Arguments start at 1

    std::string checkFunc = LUA_CHECK_FUNCS[type];
    if (checkFunc != "") {
        out << "        " << type << " " << varname << " = " << checkFunc << "(L, " << std::to_string(member ? narg + 1: narg) << ");\n";
    } else {
        std::string udataname = getMtName(type);
        out << "        " << type << " " << varname << " = **(" << type << "**)luaL_checkudata(L, " << std::to_string(member ? narg + 1 : narg) << ", \"" << udataname << "\");\n";
    }
}

// Returns an expression which tests that the given argument by index matches a certain type
static void writeLuaTestArgument(std::ofstream& out, std::string type, int narg, bool member) {
    std::string varname = "arg" + std::to_string(narg);
    narg += 1; // Arguments start at 1

    std::string testFunc = LUA_TEST_FUNCS[type];
    if (testFunc != "") {
        // Arguments start at 1
        out << testFunc << "(L, " << std::to_string(member ? narg + 1 : narg) << ")";
    } else {
        std::string udataname = getMtName(type);
        out << "luaL_testudata(L, " << std::to_string(member ? narg + 1 : narg) << ", \"" << udataname << "\")";
    }
}

static void writeLuaMethodImpls(std::ofstream& out, ClassAnalysis& state) {
    std::string fqn = "" + state.name;

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
                "    " << fqn << "* this_ = *(" << fqn << "**)luaL_checkudata(L, 1, \"__mt_" << state.name << "\");\n"
                "    int n = lua_gettop(L);\n";
        out <<  "    ";
                
        // Support multiple overloads of the same function
        bool first = true;
        for (MethodAnalysis methodImpl : methodImpls) {
            if (!first) out << " else ";
            first = false;

            // Check to see if the arguments possibly match this implementation's parameter types
            out << "if (";

            // Check number of arguments
            out << "n == " << std::to_string(methodImpl.parameters.size() + 1); // Account for first argument as 'this'

            for (size_t i = 0; i < methodImpl.parameters.size(); i++) {
                out << " && ";
                writeLuaTestArgument(out, methodImpl.parameters[i].type, i, true);
            }

            out << ") {\n"; // End if condition, start if body

            for (size_t i = 0; i < methodImpl.parameters.size(); i++) {
                writeLuaGetArgument(out, methodImpl.parameters[i].type, i, true);
            }

            // Store result
            if (methodImpl.returnType != "void")
                out << "        " << methodImpl.returnType << " result = ";
            else
                out << "        ";

            // Call function
            out << "this_->" << methodImpl.functionName << "(";

            for (size_t i = 0; i < methodImpl.parameters.size(); i++) {
                std::string varname = "arg" + std::to_string(i);
                if (i != 0) out << ", ";
                out << varname;
            }

            out << ");\n"; // End function call

            // Return result
            if (methodImpl.returnType != "void") {
                out << "        " << pushLuaValue(methodImpl.returnType, "result") << ";\n";
            }

            if (methodImpl.returnType == "void")
                out << "        return 0;\n";
            else
                out << "        return 1;\n";

            out << "    }";
        }

        // No function implementation matched
        out << "\n\n    return luaL_error(L, \"No definition of function '%s' in %s matches these argument types\", \"" << name << "\", \"" << state.name << "\");\n";

        out << "}\n\n"; // End function
    }

    for (auto& [name, methodImpls] : staticMethods) {
        std::string methodFqn = getLuaMethodFqn(state.name, name);
        out <<  "static int " << methodFqn << "(lua_State* L) {\n"
                "    int n = lua_gettop(L);\n";
        out <<  "    ";
        
        // Support multiple overloads of the same function
        bool first = true;
        for (MethodAnalysis methodImpl : methodImpls) {
            if (!first) out << " else ";
            first = false;

            // Check to see if the arguments possibly match this implementation's parameter types
            out << "if (";

            // Check number of arguments
            out << "n == " << std::to_string(methodImpl.parameters.size());

            for (size_t i = 0; i < methodImpl.parameters.size(); i++) {
                out << " && ";
                writeLuaTestArgument(out, methodImpl.parameters[i].type, i, false);
            }

            out << ") {\n"; // End if condition, start if body

            // Get the arguments
            for (size_t i = 0; i < methodImpl.parameters.size(); i++) {
                writeLuaGetArgument(out, methodImpl.parameters[i].type, i, false);
            }

            // Store result
            if (methodImpl.returnType != "void")
                out << "        " << methodImpl.returnType << " result = ";
            else
                out << "        ";

            // Call function
            if (methodImpl.functionName == "__ctor")
                out << fqn << "(";
            else
                out << fqn << "::" << methodImpl.functionName << "(";

            for (size_t i = 0; i < methodImpl.parameters.size(); i++) {
                std::string varname = "arg" + std::to_string(i);
                if (i != 0) out << ", ";
                out << varname;
            }

            out << ");\n"; // End function call

            // Return result
            if (methodImpl.returnType != "void") {
                out << "        " << pushLuaValue(methodImpl.returnType, "result") << ";\n";
            }

            if (methodImpl.returnType == "void")
                out << "        return 0;\n";
            else
                out << "        return 1;\n";

            out << "    }";
        }

        // No function implementation matched
        out << "\n\n    return luaL_error(L, \"No definition of function '%s' in %s matches these argument types\", \"" << name << "\", \"" << state.name << "\");\n";

        out << "}\n\n"; // End function
    }
}

static void writeLuaValueGenerator(std::ofstream& out, ClassAnalysis& state) {
    std::string fqn = state.name;

    out <<  "static int data_" << state.name << "_gc(lua_State*);\n"
            "static int data_" << state.name << "_index(lua_State*);\n"
            "static int data_" << state.name << "_tostring(lua_State*);\n"
            "static const struct luaL_Reg " << state.name << "_metatable [] = {\n"
            "    {\"__gc\", data_" << state.name << "_gc},\n"
            "    {\"__index\", data_" << state.name << "_index},\n"
            "    {\"__tostring\", data_" << state.name << "_tostring},\n"
            "    {NULL, NULL} /* end of array */\n"
            "};\n\n";

    out << "void " << state.name << "::PushLuaValue(lua_State* L) const {\n"
           "    int n = lua_gettop(L);\n"

           "    " << fqn << "** userdata = (" << fqn << "**)lua_newuserdata(L, sizeof(" << fqn << "));\n"
           "    *userdata = new " << fqn <<  "(*this);\n"

           "    // Create the library's metatable\n"
           "    luaL_newmetatable(L, \"__mt_" << state.name << "\");\n"
           "    luaL_register(L, NULL, " << state.name << "_metatable);\n"

           "    lua_setmetatable(L, n+1);\n"
           "}\n\n";
    

    out <<  "result<Variant, LuaCastError> " << state.name << "::FromLuaValue(lua_State* L, int idx) {\n"
            "    " << fqn << "** userdata = (" << fqn << "**) luaL_testudata(L, idx, \"" << getMtName(state.name) << "\");\n"
            "    if (userdata == nullptr)\n"
            "        return LuaCastError(lua_typename(L, idx), \"" << state.name << "\");\n"
            "    return Variant(**userdata);\n"
            "}\n\n";

    // Indexing methods and properties

    out <<  "static int data_" << state.name << "_index(lua_State* L) {\n"
            "    " << fqn << "* this_ = *(" << fqn << "**)luaL_checkudata(L, 1, \"__mt_" << state.name << "\");\n"
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
        out << "        " << pushLuaValue(type, valueExpr) << ";\n";
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

    out <<  "\nint data_" << state.name << "_tostring(lua_State* L) {\n"
    "    " << fqn << "* this_ = *(" << fqn << "**)luaL_checkudata(L, 1, \"__mt_" << state.name << "\");\n"
    "    lua_pushstring(L, std::string(this_->ToString()).c_str());\n"
    "    return 1;\n"
    "}\n\n";

    // Destructor

    out <<  "\nint data_" << state.name << "_gc(lua_State* L) {\n"
            "    " << fqn << "** userdata = (" << fqn << "**)luaL_checkudata(L, 1, \"__mt_" << state.name << "\");\n"
            "    delete *userdata;\n"
            "    return 0;\n"
            "}\n\n";
}

static void writeLuaLibraryGenerator(std::ofstream& out, ClassAnalysis& state) {
    std::string fqn = state.name;

    // If there are no static methods or properties, no need to create a library
    if (state.staticMethods.size() == 0 && state.staticProperties.size() == 0) return;

    out <<  "static int lib_" << state.name << "_index(lua_State*);\n"
            "static int lib_" << state.name << "_tostring(lua_State*);\n"
            "static const struct luaL_Reg lib_" << state.name << "_metatable [] = {\n"
            "    {\"__index\", lib_" << state.name << "_index},\n"
            "    {\"__tostring\", lib_" << state.name << "_tostring},\n"
            "    {NULL, NULL} /* end of array */\n"
            "};\n\n";

    out << "void " << state.name << "::PushLuaLibrary(lua_State* L) {\n"
           "    lua_getglobal(L, \"_G\");\n"
           "    lua_pushstring(L, \"" << state.name << "\");\n"
           "\n"
           "    lua_newuserdata(L, 0);\n"
           "\n"
           "    // Create the library's metatable\n"
           "    luaL_newmetatable(L, \"__mt_lib_" << state.name << "\");\n"
           "    luaL_register(L, NULL, lib_" << state.name << "_metatable);\n"
           "    lua_setmetatable(L, -2);\n"
           "\n"
           "    lua_rawset(L, -3);\n"
           "    lua_pop(L, 1);\n"
           "}\n\n";

    // tostring

    out <<  "\nint lib_" << state.name << "_tostring(lua_State* L) {\n"
        "    lua_pushstring(L, \"" << state.name << "\");\n"
        "    return 1;\n"
        "}\n\n";
    
    // Indexing methods and properties

    out <<  "static int lib_" << state.name << "_index(lua_State* L) {\n"
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

        out << "        " << pushLuaValue(type, valueExpr) << ";\n";
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
    out << "#define __AUTOGEN_EXTRA_INCLUDES__\n";
    out << "#include \"" << headerPath << "\"\n\n";
    out << "#include \"datatypes/variant.h\"\n";
    out << "#include <pugixml.hpp>\n";
    out << "#include \"lua.h\"\n\n";
    out << "const TypeDesc " << state.name << "::TYPE = {\n"
        << "    .name = \"" << state.serializedName << "\",\n";
    if (state.isSerializable) {
        out << "    .serialize = toVariantFunction(&" << state.name << "::Serialize),";
        if (state.hasGenericDeserializer)
            out << "    .deserialize = toVariantGenerator(&" << state.name << "::Deserialize),\n";
        else
            out << "    .deserialize = toVariantGeneratorNoMeta(&" << state.name << "::Deserialize),\n";
    }
    out << "    .toString = toVariantFunction(&" << state.name << "::ToString),";
    if (state.hasFromString) {
        if (state.hasGenericFromString)
            out << "    .fromString = toVariantGenerator(&" << state.name << "::FromString),\n";
        else
            out << "    .fromString = toVariantGeneratorNoMeta(&" << state.name << "::FromString),\n";
    }
    out << "    .pushLuaValue = toVariantFunction(&" << state.name << "::PushLuaValue),"
        << "    .fromLuaValue = toVariantGenerator(&" << state.name << "::FromLuaValue),\n"
        << "};\n\n";

    writeLuaMethodImpls(out, state);
    writeLuaValueGenerator(out, state);
    writeLuaLibraryGenerator(out, state);
}