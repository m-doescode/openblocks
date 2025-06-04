#pragma once

#include "analysis.h"
#include <fstream>

namespace enum_ {

void writeCodeForClass(std::ofstream& out, std::string headerPath, EnumAnalysis& state);

}