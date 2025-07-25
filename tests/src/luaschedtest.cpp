#include "testutillua.h"

#include "timeutil.h"

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

    test_wait1(m);
    test_wait0(m);
    test_delay(m);

    return TEST_STATUS;
}