#include "codegen.h"
#include "analysis.h"
#include <fstream>
#include <string>
#include <vector>

using namespace enum_;

void enum_::writeCodeForClass(std::ofstream& out, std::string headerPath, EnumAnalysis& state) {
    out << "#include \"datatypes/enum.h\"\n\n";

    out << "static std::pair<int, std::string> __values_" << state.name << "[] = {\n";

    for (auto entry : state.entries) {
        out << "    { " << entry.value << ", \"" << entry.name << "\" },\n";
    }

    out << "};\n\n";

    out << "static _EnumData __data_" << state.name << " = {\n"
        << "    \"" << state.name << "\",\n"
        << "    __values_" << state.name << ",\n"
        << "    " << state.entries.size() << "\n"
        << "};\n\n";

    out << "namespace EnumType {\n"
        // extern is necessary here too to prevent "const" from marking Enum as implicitly static
        // https://stackoverflow.com/questions/2190919/mixing-extern-and-const#comment2509591_2190981
        << "    extern const Enum " << state.name << "(&__data_" << state.name << ");\n"
        << "}\n\n";
}