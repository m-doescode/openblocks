#pragma once

#include "testutil.h"

#include <sstream>

#include "logger.h"
#include "objects/datamodel.h"
#include "objects/script.h"
#include "objects/service/script/scriptcontext.h"

std::string luaEvalOut(DATAMODEL_REF m, std::string source) {
    std::stringstream out;
    Logger::initTest(&out);

    auto s = Script::New();
    m->AddChild(s);
    s->source = source;
    s->Run();

    Logger::initTest(nullptr);
    return out.str();
}

void luaEval(DATAMODEL_REF m, std::string source) {
    auto s = Script::New();
    m->AddChild(s);
    s->source = source;
    s->Run();
}