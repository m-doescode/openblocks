#pragma once

#include "analysis.h"
#include <fstream>

namespace object {

void writeCodeForClass(std::ofstream& out, ClassAnalysis& state);

}