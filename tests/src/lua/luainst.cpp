#include <catch2/catch_test_macros.hpp>

#include "objects/model.h"
#include "objects/part/part.h"
#include "testcommon.h"
#include "testutil.h"

static auto& m = gTestModel;
static auto& out = testLogOutput;
// static auto& ctx = m->GetService<ScriptContext>();

TEST_CASE("Access instances") {
    SECTION("Basic access") {
        auto model = Model::New();
        m->AddChild(model);
        
        REQUIRE(luaEvalOut(m, "print(game.Model ~= nil)") == "INFO: true\n");
        REQUIRE(luaEvalOut(m, "print(workspace.Parent.Model ~= nil)") == "INFO: true\n");
        REQUIRE(luaEvalOut(m, "print(game.Model.Parent.Model ~= nil)") == "INFO: true\n");
    }
    
    SECTION("Property comes first") {
        auto model = Model::New();
        model->name = "Parent";
        m->AddChild(model);
        
        REQUIRE(luaEvalOut(m, "print(game.Parent == nil)") == "INFO: true\n");
    }
    
    SECTION("Reading properties") {
        auto part = Part::New();
        part->transparency = 2.f;
        part->anchored = true;
        part->cframe = CFrame() + Vector3(-2, 5, 3);
        part->UpdateProperty("CFrame");
        m->AddChild(part);
        
        REQUIRE(luaEvalOut(m, "print(game.Part)") == "INFO: Part\n"); // tostring
        REQUIRE(luaEvalOut(m, "print(game.Part.Name)") == "INFO: Part\n");
        REQUIRE(luaEvalOut(m, "print(game.Part.Transparency)") == "INFO: 2\n");
        REQUIRE(luaEvalOut(m, "print(game.Part.Anchored)") == "INFO: true\n");
        REQUIRE(luaEvalOut(m, "print(game.Part.Position)") == "INFO: -2, 5, 3\n");
    }
    
    SECTION("Writing properties") {
        auto part = Part::New();
        m->AddChild(part);
        
        std::string out = luaEvalOut(m, R"(
local part = game.Part
part.Name = "Some name"
part.Transparency = 1.0
part.Anchored = true
part.Position = Vector3.new(2, 3, 4)
)");

        // No error
        REQUIRE(out == "");
        REQUIRE(part->name == "Some name");
        REQUIRE(part->transparency == 1.0);
        REQUIRE(part->anchored == true);
        REQUIRE(part->position() == Vector3(2, 3, 4));
    }

    SECTION("Re-parenting") {
        auto part = Part::New();
        part->SetParent(m->FindFirstChild("Workspace"));
        REQUIRE(luaEvalOut(m, "print(workspace.Part)") == "INFO: Part\n");
        luaEval(m, "workspace.Part.Parent = game");
        REQUIRE(luaEvalOut(m, "print(game.Part)") == "INFO: Part\n");
    }
}