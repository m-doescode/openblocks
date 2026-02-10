#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "objectmodel/type.h"
#include "objectmodel/macro.h"
#include "datatypes/primitives.h"
#include "objects/base/member.h"

class TestInstance : public Instance2 {
public:
    int x;
    std::string y;

    TestInstance() {
    }
private:
    static const InstanceType2 __buildType() {
        return make_instance_type<TestInstance>(
            "TestInstance",
            
            def_property("x", &TestInstance::x),
            def_property("y", &TestInstance::y, PROP_NOSAVE)
        );
    }

    INSTANCE_HEADER_SOURCE
};

TEST_CASE("Introspection") {
    SECTION("Name") {
        REQUIRE(TestInstance::Type().className == "TestInstance");
    }

    SECTION("Property access") {
        auto testInstance = std::make_shared<TestInstance>();
        testInstance->x = 123;
        testInstance->y = "Hello, world!";

        auto type = testInstance->GetType();
        REQUIRE(type.properties.size() == 2);
        
        auto xProp = type.properties["x"];
        REQUIRE(xProp.name == "x");
        REQUIRE(xProp.getter(testInstance).GetType() == &INT_TYPE);
        REQUIRE(xProp.getter(testInstance).get<int>() == 123);
        
        auto yProp = type.properties["y"];
        REQUIRE(yProp.name == "y");
        REQUIRE(yProp.getter(testInstance).GetType() == &STRING_TYPE);
        REQUIRE(yProp.getter(testInstance).get<std::string>() == "Hello, world!");

        xProp.setter(testInstance, 456);
        yProp.setter(testInstance, "Foo, bar!");

        REQUIRE(testInstance->x == 456);
        REQUIRE(testInstance->y == "Foo, bar!");
    }

    SECTION("Property info") {
        auto type = TestInstance::Type();
        auto xProp = type.properties["x"];
        auto yProp = type.properties["y"];

        REQUIRE(xProp.type.descriptor == &INT_TYPE);
        REQUIRE(yProp.type.descriptor == &STRING_TYPE);

        REQUIRE(xProp.flags == 0);
        REQUIRE(yProp.flags == PROP_NOSAVE);
    }
}