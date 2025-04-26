#include "codegen.h"
#include "analysis.h"
#include <map>
#include <string>
#include <variant>

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

void writePropertySetHandler(std::ofstream& out, ClassAnalysis state) {
    out << "fallible<MemberNotFound, AssignToReadOnlyMember> " << state.name << "::InternalSetPropertyValue(std::string name, Data::Variant value) {";

    out << "\n    ";
    bool first = true;
    for (auto& prop : state.properties) {
        out << (first ? "" : " else ") << "if (name == \"" << prop.name << "\") {";

        if (prop.flags & PropertyFlag_Readonly) {
            out << "\n        return AssignToReadOnlyMember(\"" << state.name << "\", name)";
        } else {
            out << "\n        this->" << prop.fieldName << " = " << castFromVariant("value", prop.backingFieldType) << ";";
        }

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
}