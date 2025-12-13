#include <catch2/catch_test_macros.hpp>

#include "objects/part/part.h"
#include "objects/service/script/scriptcontext.h"
#include "objects/service/workspace.h"
#include "testcommon.h"
#include "testutil.h"
#include "timeutil.h"

static auto& m = gTestModel;
static auto& out = testLogOutput;

TEST_CASE("Connect to event") {
    auto ws = m->GetService<Workspace>();
    auto part = Part::New();
    ws->AddChild(part);

    luaEval(m, "workspace.Part.Touched:Connect(function() print('Fired!') end)");

    SECTION("Single fire") {
        part->Touched->Fire();
        REQUIRE(out.str() == "INFO: Fired!\n");
    }

    SECTION("Double fire") {
        part->Touched->Fire();
        part->Touched->Fire();
        REQUIRE(out.str() == "INFO: Fired!\nINFO: Fired!\n");
    }
}

TEST_CASE("Wait within event listener") {
    auto ctx = m->GetService<ScriptContext>();
    auto ws = m->GetService<Workspace>();
    auto part = Part::New();
    ws->AddChild(part);

    tu_set_override(0);
    luaEval(m, "workspace.Part.Touched:Connect(function() print('Fired!') wait(1) print('Waited') end)");

    SECTION("Single fire") {
        part->Touched->Fire();
        REQUIRE(out.str() == "INFO: Fired!\n");
        TT_ADVANCETIME(0.5);
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "INFO: Fired!\n");
        TT_ADVANCETIME(0.5);
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "INFO: Fired!\nINFO: Waited\n");
    }

    SECTION("Nested double fire") {
        part->Touched->Fire();
        TT_ADVANCETIME(0.2);
        part->Touched->Fire();
        REQUIRE(out.str() == "INFO: Fired!\nINFO: Fired!\n");
        TT_ADVANCETIME(1-0.2); // Small extra delay is necessary because floating point math
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "INFO: Fired!\nINFO: Fired!\nINFO: Waited\n");
        TT_ADVANCETIME(0.2);
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "INFO: Fired!\nINFO: Fired!\nINFO: Waited\nINFO: Waited\n");
    }
}

TEST_CASE("Wait for event") {
    auto ctx = m->GetService<ScriptContext>();
    auto ws = m->GetService<Workspace>();
    auto part = Part::New();
    ws->AddChild(part);

    tu_set_override(0);
    luaEval(m, "workspace.Part.Touched:Wait() print('Fired!')");

    part->Touched->Fire();
    REQUIRE(out.str() == "INFO: Fired!\n");
    part->Touched->Fire(); // Firing again should not affect output
    REQUIRE(out.str() == "INFO: Fired!\n");
}
