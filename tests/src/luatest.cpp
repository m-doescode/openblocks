#include "objects/service/script/scriptcontext.h"
#include "testutil.h"

#include "logger.h"
#include "objects/datamodel.h"
#include "objects/script.h"
#include "timeutil.h"
#include <memory>
#include <sstream>

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

void test_output(DATAMODEL_REF m) {
    ASSERT_EQ("INFO: Hello, world!\n", luaEvalOut(m, "print('Hello, world!')"));
    // ASSERT_EQ("WARN: Some warning here.\n", luaEvalOut(m, "warn('Some warning here.')"));
    // ASSERT_EQ("ERROR: An error!.\n", luaEvalOut(m, "error('An error!')"));
}

void test_wait1(DATAMODEL_REF m) {
    auto ctx = m->GetService<ScriptContext>();
    std::stringstream out;
    Logger::initTest(&out);

    tu_set_override(0);
    luaEval(m, "wait(1) print('Wait')");
    
    ctx->RunSleepingThreads();
    ASSERT_EQ("", out.str());

    TT_ADVANCETIME(0.5);
    ctx->RunSleepingThreads();
    ASSERT_EQ("", out.str());
    
    TT_ADVANCETIME(0.5);
    ctx->RunSleepingThreads();
    ASSERT_EQ("INFO: Wait\n", out.str());

    Logger::initTest(nullptr);
}

void test_wait0(DATAMODEL_REF m) {
    auto ctx = m->GetService<ScriptContext>();
    std::stringstream out;
    Logger::initTest(&out);

    tu_set_override(0);
    luaEval(m, "wait(0) print('Wait')");
    ASSERT_EQ("", out.str());
    
    ctx->RunSleepingThreads();
    ASSERT_EQ("", out.str());

    TT_ADVANCETIME(0.03);
    ctx->RunSleepingThreads();
    ASSERT_EQ("INFO: Wait\n", out.str());

    Logger::initTest(nullptr);
}

void test_delay(DATAMODEL_REF m) {
    auto ctx = m->GetService<ScriptContext>();
    std::stringstream out;
    Logger::initTest(&out);

    tu_set_override(0);
    luaEval(m, "delay(1, function() print('Delay') end)");
    
    ctx->RunSleepingThreads();
    ASSERT_EQ("", out.str());

    TT_ADVANCETIME(0.5);
    ctx->RunSleepingThreads();
    ASSERT_EQ("", out.str());
    
    TT_ADVANCETIME(0.5);
    ctx->RunSleepingThreads();
    ASSERT_EQ("INFO: Delay\n", out.str());

    Logger::initTest(nullptr);
}

int main() {
    auto m = DataModel::New();
    m->Init(true);

    test_output(m);
    test_wait1(m);
    test_wait0(m);
    test_delay(m);

    return TEST_STATUS;
}