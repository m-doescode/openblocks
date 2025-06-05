#include "analysis.h"
#include "../util.h"
#include <cctype>
#include <clang-c/CXFile.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/Index.h>
#include <cstdio>
#include <optional>

using namespace data;

static std::string toStaticName(std::string orig) {
    std::string newName = "";
    int wordStart = 0;
    for (char c : orig) {
        if (c == '_') {
            wordStart = 1;
            continue;
        }

        if (wordStart == 1)
            newName += std::toupper(c);
        else if (wordStart == 0)
            newName += std::tolower(c);
        else if (wordStart == 2)
            newName += c;

        if (c >= 'a' && c <= 'z')
            wordStart = 2;
        else
            wordStart = 0;
    }

    newName[0] = std::tolower(newName[0]);
    return newName;
}

// Constructors are stored the same way as static functions, but with the name "new"
static void processConstructor(CXCursor cur, ClassAnalysis* state) {
    std::optional<std::string> propertyDef = findAnnotation(cur, "OB::def_data_ctor");
    if (!propertyDef) return;

    MethodAnalysis anly;

    auto result = parseAnnotationString(propertyDef.value());
    std::string symbolName = x_clang_toString(clang_getCursorSpelling(cur));
    
    anly.name = result["name"];
    anly.functionName = "__ctor";
    anly.returnType = state->name;

    // if name field is not provided, use new
    if (anly.name == "") {
        anly.name = "new";
    }

    // Populate parameter list
    // https://stackoverflow.com/a/45867090/16255372
    
    for (int i = 0; i < clang_Cursor_getNumArguments(cur); i++) {
        CXCursor paramCur = clang_Cursor_getArgument(cur, i);

        std::string paramName = x_clang_toString(clang_getCursorDisplayName(paramCur));
        std::string paramType = x_clang_toString(clang_getTypeSpelling(clang_getCursorType(paramCur)));

        MethodParameter param;
        param.name = paramName;
        param.type = paramType;

        anly.parameters.push_back(param);
    }

    state->staticMethods.push_back(anly);
}

static void processMethod(CXCursor cur, ClassAnalysis* state) {
    std::optional<std::string> propertyDef = findAnnotation(cur, "OB::def_data_method");
    if (!propertyDef) return;

    MethodAnalysis anly;

    auto result = parseAnnotationString(propertyDef.value());
    std::string symbolName = x_clang_toString(clang_getCursorSpelling(cur));
    CXType retType = clang_getCursorResultType(cur);

    bool isStatic = clang_CXXMethod_isStatic(cur);
    
    anly.name = result["name"];
    anly.functionName = symbolName;
    anly.returnType = x_clang_toString(clang_getTypeSpelling(retType));

    // if name field is not provided, use fieldName instead, but capitalize the first character
    if (anly.name == "") {
        anly.name = symbolName;
        if (!isStatic)
            anly.name[0] = std::toupper(anly.name[0]);
        else
            anly.name[0] = std::tolower(anly.name[0]);
    }

    // Populate parameter list
    // https://stackoverflow.com/a/45867090/16255372
    
    for (int i = 0; i < clang_Cursor_getNumArguments(cur); i++) {
        CXCursor paramCur = clang_Cursor_getArgument(cur, i);

        std::string paramName = x_clang_toString(clang_getCursorDisplayName(paramCur));
        std::string paramType = x_clang_toString(clang_getTypeSpelling(clang_getCursorType(paramCur)));

        MethodParameter param;
        param.name = paramName;
        param.type = paramType;

        anly.parameters.push_back(param);
    }

    // If it's a static method, push it into the library instead
    if (clang_CXXMethod_isStatic(cur))
        state->staticMethods.push_back(anly);
    else
        state->methods.push_back(anly);
}

// This processes both methods and fields
static void processProperty(CXCursor cur, ClassAnalysis* state) {
    std::optional<std::string> propertyDef = findAnnotation(cur, "OB::def_data_prop");
    if (!propertyDef) return;

    PropertyAnalysis anly;

    auto result = parseAnnotationString(propertyDef.value());
    std::string symbolName = x_clang_toString(clang_getCursorSpelling(cur));
    CXType retType = clang_getCursorResultType(cur);

    bool isStatic = clang_getCursorKind(cur) == CXCursor_VarDecl || clang_CXXMethod_isStatic(cur);
    
    anly.name = result["name"];
    anly.backingSymbol = symbolName;
    anly.valueType = x_clang_toString(clang_getTypeSpelling(retType));
    anly.backingType = clang_getCursorKind(cur) == CXCursor_CXXMethod ? PropertyBackingType::Method : PropertyBackingType::Field;

    // if name field is not provided, use fieldName instead, but capitalize the first character
    if (anly.name == "") {
        anly.name = symbolName;
        if (!isStatic)
            anly.name[0] = std::toupper(anly.name[0]);
        else
            anly.name = toStaticName(anly.name);
    }

    // If it's a static property, push it into the library instead
    if (isStatic)
        state->staticProperties.push_back(anly);
    else
        state->properties.push_back(anly);
}

static bool hasMethod(CXCursor cur, std::string methodName) {
    bool found = false;
    x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind != CXCursor_CXXMethod) return CXChildVisit_Continue;
        
        if (x_clang_toString(clang_getCursorSpelling(cur)) == methodName) {
            found = true;
            return CXChildVisit_Break;
        }

        return CXChildVisit_Continue;
    });
    return found;
}

static bool hasGenericMethod(CXCursor cur, std::string methodName) {
    bool found = false;
    x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind != CXCursor_CXXMethod) return CXChildVisit_Continue;
        
        if (x_clang_toString(clang_getCursorSpelling(cur)) != methodName) return CXChildVisit_Continue;

        int numArgs = clang_Cursor_getNumArguments(cur);
        CXCursor lastParam = clang_Cursor_getArgument(cur, numArgs - 1);
        std::string lastParamType = x_clang_toString(clang_getTypeSpelling(clang_getCursorType(lastParam)));
        if (lastParamType != "const TypeMeta") return CXChildVisit_Continue;

        found = true;
        return CXChildVisit_Break;
    });
    return found;
}

static void processClass(CXCursor cur, AnalysisState* state, std::string className, std::string srcRoot) {
    ClassAnalysis anly;

    std::string propertyDef = findAnnotation(cur, "OB::def_data").value();
    auto result = parseAnnotationString(propertyDef);

    anly.name = className;
    anly.serializedName = result["name"];
    anly.hasFromString = hasMethod(cur, "FromString");
    anly.isSerializable = hasMethod(cur, "Serialize") && hasMethod(cur, "Deserialize");
    anly.hasGenericDeserializer = hasGenericMethod(cur, "Deserialize");
    anly.hasGenericFromString = hasGenericMethod(cur, "FromString");

    if (anly.serializedName == "")
        anly.serializedName = className;
    
    x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);

        if (kind == CXCursor_Constructor) {
            processConstructor(cur, &anly);
        }
        
        if (kind == CXCursor_CXXMethod || kind == CXCursor_FieldDecl || kind == CXCursor_VarDecl) {
            processProperty(cur, &anly);
        }

        if (kind == CXCursor_CXXMethod) {
            processMethod(cur, &anly);
        }

        return CXChildVisit_Continue;
    });

    state->classes[className] = anly;
}

bool data::analyzeClasses(CXCursor cursor, std::string srcRoot, AnalysisState* state) {
    // Search for classes
    x_clang_visitChildren(cursor, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind == CXCursor_Namespace) return CXChildVisit_Recurse;
        if (kind != CXCursor_ClassDecl) return CXChildVisit_Continue;

        CXSourceLocation loc = clang_getCursorLocation(cur);
        if (!clang_Location_isFromMainFile(loc)) return CXChildVisit_Continue; // This class is not from this header. Skip

        std::string className = x_clang_toString(clang_getCursorDisplayName(cur));
        // Forward-decls can slip through the cracks, this prevents that, but also allows us to filter non-instance classes in the src/objects directory
        if (!findAnnotation(cur, "OB::def_data")) return CXChildVisit_Continue; // Class is not "primary" declaration/is not instance, skip
        if (state->classes.count(className) > 0) return CXChildVisit_Continue; // Class has already been analyzed, skip...

        processClass(cur, state, className, srcRoot);

        return CXChildVisit_Continue;
    });

    return true;
}