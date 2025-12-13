#include <catch2/catch_test_macros.hpp>

#include "testcommon.h"
#include "testutil.h"

TEST_CASE("Generic lua test", "[luageneric]") {
    auto m = gTestModel;
    
    SECTION("Script output") {
        REQUIRE(luaEvalOut(m, "print('Hello, world!')") == "INFO: Hello, world!\n");
    }
    
    // SECTION("Script warning") {
    //     REQUIRE(luaEvalOut(m, "warn('Some warning here.')") == "WARN: Some warning here.\n");
    // }
    
    // SECTION("Script error") {
    //     REQUIRE(luaEvalOut(m, "error('An error!')") == "ERROR: An error!.\n");
    // }
}