#include "codegen.h"
#include "analysis.h"

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
}