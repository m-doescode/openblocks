#pragma once

#include "objects/datamodel.h"
#include "objects/script.h"
#include "objects/service/script/serverscriptservice.h"
#include "testcommon.h"

#define TU_TIME_EXPOSE_TEST
#define TT_ADVANCETIME(secs) tu_set_override(tu_clock_micros() + (secs) * 1'000'000);

inline std::string luaEvalOut(std::shared_ptr<DataModel> m, std::string source) {
    size_t offset = testLogOutput.tellp();
    testLogOutput.seekp(0, std::ios::end);

    auto ss = m->GetService<ServerScriptService>();
    auto s = Script::New();
    m->AddChild(s);
    s->source = source;
    s->Run();

    return testLogOutput.str().substr(offset);
}

inline void luaEval(std::shared_ptr<DataModel> m, std::string source) {
    auto ss = m->GetService<ServerScriptService>();
    auto s = Script::New();
    ss->AddChild(s);
    s->source = source;
    s->Run();
}
