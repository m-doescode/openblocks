#pragma once

#include "analysis.h"
#include <fstream>

namespace data {

void writeCodeForClass(std::ofstream& out, std::string headerPath, ClassAnalysis& state);

}