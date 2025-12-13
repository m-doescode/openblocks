#include <catch2/catch_test_macros.hpp>

#include "objects/service/script/scriptcontext.h"
#include "testcommon.h"
#include "testutil.h"
#include "timeutil.h"

static auto& m = gTestModel;
static auto& out = testLogOutput;

TEST_CASE("Wait with delay") {
    auto ctx = m->GetService<ScriptContext>();

    tu_set_override(0);
    luaEval(m, "wait(1) print('Wait')");

    SECTION("Empty output at 0s") {
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "");
    }

    SECTION("Empty output at 0.5s") {
        TT_ADVANCETIME(0.5);
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "");
    }

    SECTION("Print output at 1s") {
        TT_ADVANCETIME(1);
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "INFO: Wait\n");
    }
}

TEST_CASE("Wait with minimum delay") {
    auto ctx = m->GetService<ScriptContext>();

    tu_set_override(0);
    luaEval(m, "wait(0) print('Wait')");

    SECTION("Empty output at 0s") {
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "");
    }

    SECTION("Empty output at 0.02s") {
        TT_ADVANCETIME(0.02);
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "");
    }

    SECTION("Print output at 0.03s") {
        TT_ADVANCETIME(0.03);
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "INFO: Wait\n");
    }
}

TEST_CASE("Run callback after delay") {
    auto ctx = m->GetService<ScriptContext>();

    tu_set_override(0);
    luaEval(m, "delay(1, function() print('Delay') end)");

    SECTION("Empty output at 0s") {
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "");
    }

    SECTION("Empty output at 0.5s") {
        TT_ADVANCETIME(0.5);
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "");
    }

    SECTION("Print output at 1s") {
        TT_ADVANCETIME(1);
        ctx->RunSleepingThreads();
        REQUIRE(out.str() == "INFO: Delay\n");
    }
}
