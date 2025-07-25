
#include "testutil.h"
#include "testutillua.h"

#include "timeutil.h"
#include "objects/part/part.h"
#include "objects/service/workspace.h"
#include <memory>
#include <sstream>

void test_connect(DATAMODEL_REF m) {
    auto ctx = m->GetService<ScriptContext>();
    auto part = Part::New();
    m->GetService<Workspace>()->AddChild(part);
    std::stringstream out;
    Logger::initTest(&out);

    luaEval(m, "workspace.Part.Touched:Connect(function() print('Fired!') end)");
    ASSERT_EQ("", out.str());

    part->Touched->Fire();
    ASSERT_EQ("INFO: Fired!\n", out.str());
    part->Touched->Fire();
    ASSERT_EQ("INFO: Fired!\nINFO: Fired!\n", out.str());

    Logger::initTest(nullptr);
    part->Destroy();
}

void test_waitwithin(DATAMODEL_REF m) {
    auto ctx = m->GetService<ScriptContext>();
    auto part = Part::New();
    m->GetService<Workspace>()->AddChild(part);
    std::stringstream out;
    Logger::initTest(&out);

    tu_set_override(0);
    luaEval(m, "workspace.Part.Touched:Connect(function() print('Fired!') wait(1) print('Waited') end)");
    ASSERT_EQ("", out.str());

    // One shot
    part->Touched->Fire();
    ctx->RunSleepingThreads();
    ASSERT_EQ("INFO: Fired!\n", out.str());
    TT_ADVANCETIME(0.5);
    ctx->RunSleepingThreads();
    ASSERT_EQ("INFO: Fired!\n", out.str());
    TT_ADVANCETIME(0.5);
    ctx->RunSleepingThreads();
    ASSERT_EQ("INFO: Fired!\nINFO: Waited\n", out.str());

    // Clear
    out = std::stringstream();
    Logger::initTest(&out); // Shouldn't *theoretically* be necessary, but just in principle...

    // Double fire
    part->Touched->Fire();
    TT_ADVANCETIME(0.2);
    part->Touched->Fire();
    ASSERT_EQ("INFO: Fired!\nINFO: Fired!\n", out.str());
    TT_ADVANCETIME(1-0.2); // Small extra delay is necessary because floating point math
    ctx->RunSleepingThreads();
    ASSERT_EQ("INFO: Fired!\nINFO: Fired!\nINFO: Waited\n", out.str());
    TT_ADVANCETIME(0.2);
    ctx->RunSleepingThreads();
    ASSERT_EQ("INFO: Fired!\nINFO: Fired!\nINFO: Waited\nINFO: Waited\n", out.str());

    tu_set_override(-1UL);
    Logger::initTest(nullptr);
    part->Destroy();
}

void test_await(DATAMODEL_REF m) {
    auto ctx = m->GetService<ScriptContext>();
    auto part = Part::New();
    m->GetService<Workspace>()->AddChild(part);
    std::stringstream out;
    Logger::initTest(&out);

    tu_set_override(0);
    luaEval(m, "workspace.Part.Touched:Wait() print('Fired!')");
    ASSERT_EQ("", out.str());

    part->Touched->Fire();
    ASSERT_EQ("INFO: Fired!\n", out.str());
    part->Touched->Fire(); // Firing again should not affect output
    ASSERT_EQ("INFO: Fired!\n", out.str());

    tu_set_override(-1UL);
    Logger::initTest(nullptr);
    part->Destroy();
}

int main() {
    auto m = DataModel::New();
    m->Init(true);

    test_connect(m);
    test_waitwithin(m);
    test_await(m);

    return TEST_STATUS;
}