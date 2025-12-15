#include "objects/datamodel.h"
#include "objects/script.h"
#include <catch2/catch_test_macros.hpp>

extern int _dbgDataModelDestroyCount;

TEST_CASE("Datamodel destruction") {
    // Ensure no cyclic-dependency causing datamodel to not be destructed
    
    auto root = DataModel::New();
    root->Init(true);
    
    SECTION("Empty model") {
        int prevCount = _dbgDataModelDestroyCount;
        root = nullptr;
        REQUIRE(_dbgDataModelDestroyCount == prevCount + 1);
    }
    
    SECTION("Model with script") {
        auto s = Script::New();
        root->AddChild(s);
        s->source = "local x = game; wait(0)";
        s->Run();
        int prevCount = _dbgDataModelDestroyCount;
        root = nullptr;
        REQUIRE(_dbgDataModelDestroyCount == prevCount + 1);
    }
}