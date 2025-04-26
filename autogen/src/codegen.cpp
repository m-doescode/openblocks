#include "codegen.h"
#include "analysis.h"
#include <map>
#include <string>
#include <variant>

std::map<std::string, std::string> CATEGORY_STR = {
    { "appearance", "PROP_CATEGORY_APPEARENCE" },
    { "data", "PROP_CATEGORY_DATA" },
    { "behavior", "PROP_CATEGORY_BEHAVIOR" },
    { "part", "PROP_CATEGORY_PART" },
    { "surface", "PROP_CATEGORY_SURFACE" },
};

std::map<std::string, std::string> MAPPED_TYPE = {
    { "bool", "Data::Bool" },
    { "int", "Data::Int" },
    { "float", "Data::Float" },
    { "std::string", "Data::String" },
    { "std::weak_ptr<Instance>", "Data::InstanceRef" },
    { "glm::vec3", "Vector3" },
};

std::map<std::string, std::monostate> ENUM_TYPES = {
    { "SurfaceType", std::monostate() }
};

std::string castFromVariant(std::string valueStr, std::string fieldType) {
    // Manual exception for now, enums will get their own system eventually
    if (fieldType == "SurfaceType") {
        return "(SurfaceType)(int)" + valueStr + ".get<Data::Int>()";
    }

    std::string mappedType = MAPPED_TYPE[fieldType];
    return valueStr + ".get<" + (!mappedType.empty() ? mappedType : fieldType) + ">()";
}

std::string castToVariant(std::string valueStr, std::string fieldType) {
    // Manual exception for now, enums will get their own system eventually
    if (fieldType == "SurfaceType") {
        return "Data::Int((int)" + valueStr + ")";
    }

    std::string mappedType = MAPPED_TYPE[fieldType];
    if (!mappedType.empty()) {
        return mappedType + "(" + valueStr + ")";
    }
    return valueStr;
}

void writePropertySetHandler(std::ofstream& out, ClassAnalysis state) {
    out << "fallible<MemberNotFound, AssignToReadOnlyMember> " << state.name << "::InternalSetPropertyValue(std::string name, Data::Variant value) {";

    out << "\n    ";
    bool first = true;
    for (auto& prop : state.properties) {
        out << (first ? "" : " else ") << "if (name == \"" << prop.name << "\") {";

        if (prop.flags & PropertyFlag_Readonly) {
            out << "\n        return AssignToReadOnlyMember(\"" << state.name << "\", name)";
        } else if (prop.cframeMember == CFrameMember_Position) {
            out << "\n        this->" << prop.fieldName << " = this->" << prop.fieldName << ".Rotation() + value.get<Vector3>();";
        }  else if (prop.cframeMember == CFrameMember_Rotation) {
            out << "\n        this->" << prop.fieldName << " = CFrame::FromEulerAnglesXYZ(value.get<Vector3>()) + this->" << prop.fieldName << ".Position();";
        } else {
            out << "\n        this->" << prop.fieldName << " = " << castFromVariant("value", prop.backingFieldType) << ";";
            if (!prop.onUpdateCallback.empty())
                out << "\n        " << prop.onUpdateCallback << "(name);";
        }

        out << "\n    }";
        first = false;
    }
    
    out << "\n    return MemberNotFound(\"" << state.name << "\", name);";

    out << "\n};\n\n";
}

void writePropertyGetHandler(std::ofstream& out, ClassAnalysis state) {
    out << "result<Data::Variant, MemberNotFound> " << state.name << "::InternalGetPropertyValue(std::string name) {";

    out << "\n    ";
    bool first = true;
    for (auto& prop : state.properties) {
        out << (first ? "" : " else ") << "if (name == \"" << prop.name << "\") {";

        if (prop.cframeMember == CFrameMember_Position) {
            out << "\n        return Data::Variant(" << prop.fieldName << ".Position());";
        } else if (prop.cframeMember == CFrameMember_Rotation) {
            out << "\n        return Data::Variant(" << prop.fieldName << ".ToEulerAnglesXYZ());";
        } else {
            out << "\n        return Data::Variant(" << castToVariant(prop.fieldName, prop.backingFieldType) << ");";
        }

        out << "\n    }";
        first = false;
    }
    
    out << "\n    return MemberNotFound(\"" << state.name << "\", name);";

    out << "\n};\n\n";
}

void writePropertyMetaHandler(std::ofstream& out, ClassAnalysis state) {
    out << "result<PropertyMeta, MemberNotFound> " << state.name << "::InternalGetPropertyMeta(std::string name) {";

    out << "\n    ";
    bool first = true;
    for (auto& prop : state.properties) {
        out << (first ? "" : " else ") << "if (name == \"" << prop.name << "\") {";

        std::string type = MAPPED_TYPE[prop.backingFieldType];
        if (type.empty()) type = prop.backingFieldType;
        if (type == "SurfaceType") type = "Data::Int";

        std::string strFlags;
        if (prop.flags & PropertyFlag_Readonly)
            strFlags += " | PROP_READONLY";
        if (prop.flags & PropertyFlag_Hidden)
            strFlags += " | PROP_HIDDEN";
        if (prop.flags & PropertyFlag_NoSave)
            strFlags += " | PROP_NOSAVE";
        if (prop.flags & PropertyFlag_UnitFloat)
            strFlags += " | PROP_UNIT_FLOAT";
        if (!strFlags.empty()) strFlags = strFlags.substr(3); // Remove leading pipe
        else strFlags = "0"; // 0 == No option

        std::string category = CATEGORY_STR[prop.category];
        if (category.empty()) category = "PROP_CATEGORY_DATA";

        out << "\n        return PropertyMeta { &" << type << "::TYPE, " << strFlags << ", " << category << " };";

        out << "\n    }";
        first = false;
    }
    
    out << "\n    return MemberNotFound(\"" << state.name << "\", name);";

    out << "\n};\n\n";
}

void writeCodeForClass(std::ofstream& out, ClassAnalysis& state) {
    std::string strFlags;
    if (state.flags & ClassFlag_NotCreatable)
        strFlags += " | INSTANCE_NOT_CREATABLE";
    if (state.flags & ClassFlag_Service)
        strFlags += " | INSTANCE_SERVICE";
    if (state.flags & ClassFlag_Hidden)
        strFlags += " | INSTANCE_HIDDEN";
    if (!strFlags.empty()) strFlags = strFlags.substr(3); // Remove leading pipe
    else strFlags = "0"; // 0 == No option

    out << "/////////////////////////////////////////////////////////////////////////////////////////\n";
    out << "// This file was automatically generated by autogen, and should not be edited manually //\n";
    out << "/////////////////////////////////////////////////////////////////////////////////////////\n\n";

    out << "#include \"" << state.headerPath << "\"\n\n";
    out << "const InstanceType " << state.name << "::TYPE = {\n"
        << "    .super = &" << state.baseClass << "::TYPE,\n"
        << "    .className = \"" << state.name << "\",\n"
        << "    .constructor = &" << state.name << "::CreateGeneric,\n"
        << "    .explorerIcon = \"" << state.explorerIcon << "\",\n"
        << "    .flags = " << strFlags << ",\n"
        << "};\n\n";

    out << "const InstanceType* " << state.name << "::GetClass() {\n"
        << "    return &TYPE;\n"
        << "};\n\n";

    writePropertySetHandler(out, state);
    writePropertyGetHandler(out, state);
    writePropertyMetaHandler(out, state);
}