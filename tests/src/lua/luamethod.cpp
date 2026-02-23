#include <catch2/catch_test_macros.hpp>

#include "objects/service/workspace.h"
#include "objects/part/part.h"
#include "testcommon.h"
#include "testutil.h"


static auto& m = gTestModel;
static auto& out = testLogOutput;

// Gets only the first outputted line
inline std::string luaEvalOutLn(std::shared_ptr<DataModel> m, std::string source) {
    std::string out = luaEvalOut(m, source);
    return out.substr(0, out.find('\n')+1);
}

TEST_CASE("Lua instance methods", "[luamethod]") {
    auto ws = m->GetService<Workspace>();
    auto part = Part::New();
    ws->AddChild(part);
    
    SECTION("Calling methods") {
        REQUIRE(luaEvalOut(m, "print(workspace.Part:Clone())") == "INFO: Part\n");
        REQUIRE(luaEvalOut(m, "print(workspace:FindFirstChild('Part'))") == "INFO: Part\n");
    }
}