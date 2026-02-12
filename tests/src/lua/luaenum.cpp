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

TEST_CASE("Lua enums", "[luaenum]") {
    auto ws = m->GetService<Workspace>();
    auto part = Part::New();
    ws->AddChild(part);
    
    SECTION("Accessing enums") {
        REQUIRE(luaEvalOut(m, "print(Enum.PartType)") == "INFO: PartType\n");
        REQUIRE(luaEvalOut(m, "print(Enum.PartType.Ball)") == "INFO: Enum.PartType.Ball\n");
        REQUIRE(luaEvalOut(m, "print(Enum.PartType.Ball.Name)") == "INFO: Ball\n");
        REQUIRE(luaEvalOut(m, "print(Enum.PartType.Ball.Value)") == "INFO: 0\n");
        REQUIRE(luaEvalOut(m, "print(Enum.PartType.Ball.EnumType)") == "INFO: PartType\n");

        REQUIRE(luaEvalOut(m, "print(#Enum.PartType:GetEnumItems())") == "INFO: 3\n");
        REQUIRE(luaEvalOut(m, "print(Enum.PartType:GetEnumItems()[1])") == "INFO: Enum.PartType.Ball\n");
        REQUIRE(luaEvalOut(m, "print(Enum.PartType:FromName('Ball')") == "INFO: Enum.PartType.Ball\n");
        REQUIRE(luaEvalOut(m, "print(Enum.PartType:FromValue(0)") == "INFO: Enum.PartType.Ball\n");
        REQUIRE(luaEvalOut(m, "print(Enum.PartType:FromName('Nonexistent')") == "INFO: nil\n");
        REQUIRE(luaEvalOut(m, "print(Enum.PartType:FromValue(-1)") == "INFO: nil\n");
        // TODO: Handle errors
        REQUIRE(luaEvalOutLn(m, "print(Enum.Nonexistent)") == "ERROR: Nonexistent is not a valid EnumItem\n");
        REQUIRE(luaEvalOutLn(m, "print(Enum.PartType.Nonexistent)") == "ERROR: Nonexistent is not a valid EnumItem\n");
    }
}