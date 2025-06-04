#include "codegen.h"
#include "analysis.h"
#include <fstream>
#include <map>
#include <string>
#include <variant>
#include <vector>

using namespace enum_;

void enum_::writeCodeForClass(std::ofstream& out, std::string headerPath, EnumAnalysis& state) {
    out << "#include \"datatypes/enum.h\"\n\n";

    out << "static std::pair<int, std::string> __values_" << state.name << "[] = {\n";

    for (auto entry : state.entries) {
        out << "    { " << entry.value << ", \"" << entry.name << "\" },\n";
    }

    out << "};";

    out << "static _EnumData __data_" << state.name << " = {\n"
        << "    \"" << state.name << "\",\n"
        << "    __values_" << state.name << ",\n"
        << "    " << state.entries.size() << "\n"
        << "};\n\n";

    out << "namespace EnumType {\n"
        << "    const Enum " << state.name << "(&__data_" << state.name << ");\n"
        << "}\n\n";
}