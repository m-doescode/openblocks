// Basic operations such as instantiation, re-parenting, and destruction

#include "common.h"
#include "error/instance.h"
#include "objects/model.h"
#include "objects/part/part.h"
#include <catch2/catch_test_macros.hpp>

static auto& m = gDataModel;

TEST_CASE("Construction") {
    auto folder = Model::New();
    m->AddChild(folder);
    
    SECTION("Constructing container") {
        bool found = false;
        for (auto& obj : m->GetChildren()) {
            if (obj == folder) {
                found = true;
            }
        }
        
        REQUIRE(found);
        REQUIRE(folder->GetParent() != nullptr);
        REQUIRE(folder->GetParent() == m);
    }
    
    SECTION("Constructing Part") {
        auto part = Part::New();
        folder->AddChild(part);
        
        REQUIRE(folder->GetChildren().size() == 1);
        REQUIRE(folder->GetChildren()[0] == part);
        REQUIRE(part->GetParent() != nullptr);
        REQUIRE(part->GetParent() == folder);
    }
    
    SECTION("Dynamic construction of part") {
        auto result = Instance::New("Part");
        REQUIRE(result.isSuccess());
    }
    
    SECTION("Invalid construction of service") {
        auto result = Instance::New("Workspace");
        REQUIRE(result.isError());
        REQUIRE(std::holds_alternative<NotCreatableInstance>(result.error().value()));
    }
    
    SECTION("Invalid construction of nonexistent type") {
        auto result = Instance::New("__INVALID");
        REQUIRE(result.isError());
        REQUIRE(std::holds_alternative<NoSuchInstance>(result.error().value()));
    }
}

TEST_CASE("Parenting") {
    auto folder = Model::New();
    m->AddChild(folder);
    
    SECTION("Reparent part to another folder") {
        auto folder2 = Model::New();
        m->AddChild(folder2);
        
        auto part = Part::New();
        folder->AddChild(part);
        
        // Verify initial folder
        REQUIRE(folder->GetChildren().size() == 1);
        REQUIRE(folder->GetChildren()[0] == part);
        REQUIRE(part->GetParent() != nullptr);
        REQUIRE(part->GetParent() == folder);
        
        folder2->AddChild(part); // AddChild just internally calls SetParent, so it should automatically take care of cleanup
        
        // Verify new folder
        REQUIRE(folder->GetChildren().size() == 0);
        REQUIRE(folder2->GetChildren().size() == 1);
        REQUIRE(folder2->GetChildren()[0] == part);
        REQUIRE(part->GetParent() != nullptr);
        REQUIRE(part->GetParent() == folder2);
    }
    
    SECTION("Unparenting") {
        auto part = Part::New();
        folder->AddChild(part);
        
        part->SetParent(nullptr);
        REQUIRE(folder->GetChildren().size() == 0);
        REQUIRE(part->GetParent() == nullptr);
    }
    
    SECTION("Nested reparent") {
        auto folder2 = Model::New();
        m->AddChild(folder2);
        
        auto part = Part::New();
        folder->AddChild(part);
        auto part2 = Part::New();
        part->AddChild(part2);
        
        REQUIRE(part->GetChildren().size() == 1);
        REQUIRE(part->GetChildren()[0] == part2);
        REQUIRE(part2->GetParent() == part);
        
        folder2->AddChild(part); // AddChild just internally calls SetParent, so it should automatically take care of cleanup
        
        // Make sure nothing changed
        REQUIRE(part->GetChildren().size() == 1);
        REQUIRE(part->GetChildren()[0] == part2);
        REQUIRE(part2->GetParent() == part);
    }
}