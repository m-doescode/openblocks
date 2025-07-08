#include "codegen.h"
#include "analysis.h"
#include <map>
#include <string>
#include <variant>

using namespace object;

static std::map<std::string, std::string> CATEGORY_STR = {
    { "APPEARANCE", "PROP_CATEGORY_APPEARENCE" },
    { "DATA", "PROP_CATEGORY_DATA" },
    { "BEHAVIOR", "PROP_CATEGORY_BEHAVIOR" },
    { "PART", "PROP_CATEGORY_PART" },
    { "SURFACE", "PROP_CATEGORY_SURFACE" },
    { "SURFACE_INPUT", "PROP_CATEGORY_SURFACE_INPUT" },
};

static std::map<std::string, std::string> MAPPED_TYPE = {
};

static std::map<std::string, std::string> TYPEINFO_REFS = {
    { "bool", "BOOL_TYPE" },
    { "int", "INT_TYPE" },
    { "float", "FLOAT_TYPE" },
    { "std::string", "STRING_TYPE" },
};

static std::map<std::string, std::monostate> ENUM_TYPES = {
    { "SurfaceType", std::monostate() }
};

static std::string parseWeakPtr(std::string weakPtrType) {
    if (!weakPtrType.starts_with("std::weak_ptr")) return "";

    int pos0 = weakPtrType.find("<");
    int pos1 = weakPtrType.find(">");

    std::string subtype = weakPtrType.substr(pos0+1, pos1-pos0-1);
    return subtype;
}

static std::string castFromVariant(std::string valueStr, std::string fieldType) {
    // Manual exception for now, enums will get their own system eventually
    if (fieldType == "SurfaceType") {
        return "(SurfaceType)(int)" + valueStr + ".get<int>()";
    }

    std::string mappedType = MAPPED_TYPE[fieldType];
    return "(" + fieldType + ")" + valueStr + ".get<" + (!mappedType.empty() ? mappedType : fieldType) + ">()";
}

static std::string castToVariant(std::string valueStr, std::string fieldType) {
    // Manual exception for now, enums will get their own system eventually
    if (fieldType == "SurfaceType") {
        return "(int)" + valueStr;
    }

    // std::shared_ptr<Instance>
    std::string subtype = parseWeakPtr(fieldType);
    if (!subtype.empty()) {
        return "Variant(" + valueStr + ".expired() ? InstanceRef() : InstanceRef(std::dynamic_pointer_cast<Instance>(" + valueStr + ".lock())))";
    }

    std::string mappedType = MAPPED_TYPE[fieldType];
    if (!mappedType.empty()) {
        return mappedType + "(" + valueStr + ")";
    }
    return valueStr;
}

static void writePropertySetHandler(std::ofstream& out, ClassAnalysis state) {
    out << "fallible<MemberNotFound, AssignToReadOnlyMember> " << state.name << "::InternalSetPropertyValue(std::string name, Variant value) {";

    out << "\n    ";
    bool first = true;
    for (auto& prop : state.properties) {
        out << (first ? "" : " else ") << "if (name == \"" << prop.name << "\") {";
        // std::shared_ptr<Instance>
        std::string subtype = parseWeakPtr(prop.backingFieldType);

        if (prop.flags & PropertyFlag_Readonly) {
            out << "\n        return AssignToReadOnlyMember(GetClass()->className, name);";
        } else if (prop.cframeMember == CFrameMember_Position) {
            out << "\n        this->" << prop.fieldName << " = this->" << prop.fieldName << ".Rotation() + value.get<Vector3>();";
        }  else if (prop.cframeMember == CFrameMember_Rotation) {
            out << "\n        this->" << prop.fieldName << " = CFrame::FromEulerAnglesXYZ(value.get<Vector3>()) + this->" << prop.fieldName << ".Position();";
        } else if (!subtype.empty()) {
            out << "\n        std::weak_ptr<Instance> ref = value.get<InstanceRef>();"
                << "\n        this->" << prop.fieldName << " = ref.expired() ? std::weak_ptr<" << subtype << ">() : std::dynamic_pointer_cast<" << subtype << ">(ref.lock());";
        } else if (prop.backingFieldType == "EnumItem") {
            out << "\n        this->" << prop.fieldName << " = (" << prop.backingFieldEnum << ")value.get<EnumItem>().Value();";
        } else {
            out << "\n        this->" << prop.fieldName << " = " << castFromVariant("value", prop.backingFieldType) << ";";
        }

        out << "\n    }";
        first = false;
    }

    // If it's empty, just return the parent's impl
    if (state.properties.empty()) {
        out << "\n    return " << state.baseClass << "::InternalSetPropertyValue(name, value);";
    } else {
        // Otherwise, add else and return
        out << "else {";
        out << "\n        return " << state.baseClass << "::InternalSetPropertyValue(name, value);";
        out << "\n    }";
        out << "\n    return {};";
    }

    out << "\n};\n\n";
}

static void writePropertyUpdateHandler(std::ofstream& out, ClassAnalysis state) {
    out << "void " << state.name << "::InternalUpdateProperty(std::string name) {";

    out << "\n    ";
    bool first = true;
    for (auto& prop : state.properties) {
        if (prop.onUpdateCallback.empty()) continue;
        out << (first ? "" : " else ") << "if (name == \"" << prop.name << "\") {";

        out << "\n        " << prop.onUpdateCallback << "(name);";

        out << "\n    }";
        first = false;
    }

    out << "\n    " << state.baseClass << "::InternalUpdateProperty(name);";

    out << "\n};\n\n";
}

static void writePropertyGetHandler(std::ofstream& out, ClassAnalysis state) {
    out << "result<Variant, MemberNotFound> " << state.name << "::InternalGetPropertyValue(std::string name) {";

    out << "\n    ";
    bool first = true;
    for (auto& prop : state.properties) {
        out << (first ? "" : " else ") << "if (name == \"" << prop.name << "\") {";

        if (prop.cframeMember == CFrameMember_Position) {
            out << "\n        return Variant(" << prop.fieldName << ".Position());";
        } else if (prop.cframeMember == CFrameMember_Rotation) {
            out << "\n        return Variant(" << prop.fieldName << ".ToEulerAnglesXYZ());";
        } else if (prop.backingFieldType == "EnumItem") {
            out << "\n        return Variant(EnumType::" << prop.backingFieldEnum << ".FromValueInternal((int)" << prop.fieldName << "));";
        } else {
            out << "\n        return Variant(" << castToVariant(prop.fieldName, prop.backingFieldType) << ");";
        }

        out << "\n    }";
        first = false;
    }

    // Handle signals
    out << "\n    ";
    first = true;
    for (auto& signal : state.signals) {
        out << (first ? "" : " else ") << "if (name == \"" << signal.name << "\") {";

        out << "\n        return Variant(SignalRef(" << signal.sourceFieldName << "));";

        out << "\n    }";
        first = false;
    }
    
    out << "\n    return " << state.baseClass << "::InternalGetPropertyValue(name);";
    // out << "\n    return MemberNotFound(\"" << state.name << "\", name);";

    out << "\n};\n\n";
}

static void writePropertiesList(std::ofstream& out, ClassAnalysis state) {
    out << "std::vector<std::string> " << state.name << "::InternalGetProperties() {\n";
    out << "    std::vector<std::string> properties = " << state.baseClass << "::InternalGetProperties();\n";

    for (auto& prop : state.properties) {
        out << "    properties.push_back(\"" << prop.name << "\");\n";
    }

    out << "    return properties;\n";

    out << "};\n\n";
}

static void writePropertyMetaHandler(std::ofstream& out, ClassAnalysis state) {
    out << "result<PropertyMeta, MemberNotFound> " << state.name << "::InternalGetPropertyMeta(std::string name) {";

    out << "\n    ";
    bool first = true;
    for (auto& prop : state.properties) {
        out << (first ? "" : " else ") << "if (name == \"" << prop.name << "\") {";

        std::string typeInfo = TYPEINFO_REFS[prop.backingFieldType];
        if (typeInfo.empty()) typeInfo = prop.backingFieldType + "::TYPE";
        if (!parseWeakPtr(prop.backingFieldType).empty()) typeInfo = "InstanceRef::TYPE";
        if (prop.backingFieldType == "EnumItem") typeInfo = "EnumType::" + prop.backingFieldEnum;

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

        out << "\n        return PropertyMeta { &" << typeInfo << ", " << strFlags << ", " << category << " };";

        out << "\n    }";
        first = false;
    }

    // Handle signals
    out << "\n    ";
    first = true;
    for (auto& signal : state.signals) {
        out << (first ? "" : " else ") << "if (name == \"" << signal.name << "\") {";
        
        std::string strFlags;
        strFlags += "PROP_READONLY";
        strFlags += " | PROP_HIDDEN";
        strFlags += " | PROP_NOSAVE";

        out << "\n        return PropertyMeta { &SignalRef::TYPE, " << strFlags << " };";

        out << "\n    }";
        first = false;
    }
    
    out << "\n    return " << state.baseClass << "::InternalGetPropertyMeta(name);";
    // out << "\n    return MemberNotFound(\"" << state.name << "\", name);";

    out << "\n};\n\n";
}

void object::writeCodeForClass(std::ofstream& out, std::string headerPath, ClassAnalysis& state) {
    std::string strFlags;
    if (state.flags & ClassFlag_NotCreatable)
        strFlags += " | INSTANCE_NOTCREATABLE";
    if (state.flags & ClassFlag_Service)
        strFlags += " | INSTANCE_SERVICE";
    if (state.flags & ClassFlag_Hidden)
        strFlags += " | INSTANCE_HIDDEN";
    if (!strFlags.empty()) strFlags = strFlags.substr(3); // Remove leading pipe
    else strFlags = "0"; // 0 == No option

    std::string constructorStr;
    if (state.abstract) constructorStr = "nullptr";
    else constructorStr = "&" + state.name + "::Create";

    out << "#define __AUTOGEN_EXTRA_INCLUDES__\n";
    out << "#include \"" << state.headerPath << "\"\n\n";
    out << "#include \"datatypes/variant.h\"\n";
    out << "#include \"datatypes/primitives.h\"\n";
    out << "const InstanceType " << state.name << "::TYPE = {\n"
        << "    .super = &" << state.baseClass << "::TYPE,\n"
        << "    .className = \"" << state.name << "\",\n"
        << "    .constructor = " << constructorStr << ",\n"
        << "    .explorerIcon = \"" << state.explorerIcon << "\",\n"
        << "    .flags = " << strFlags << ",\n"
        << "};\n\n";

    out << "const InstanceType* " << state.name << "::GetClass() {\n"
        << "    return &TYPE;\n"
        << "};\n\n";

    // Special case for our Service class
    if (state.baseClass == "Service") state.baseClass = "Instance";

    writePropertySetHandler(out, state);
    writePropertyGetHandler(out, state);
    writePropertyMetaHandler(out, state);
    writePropertyUpdateHandler(out, state);
    writePropertiesList(out, state);
}