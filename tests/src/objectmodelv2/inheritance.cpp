#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "objectmodel/macro.h"
#include "objectmodel/property.h"
#include "objectmodel/type.h"
#include "objects/base/instance.h"

class TestBaseInstance : public Instance {
public:
    int x;
    int y;
private:
    static InstanceType __buildType() {
        return make_instance_type<TestBaseInstance>(
            "TestBaseInstance",

            def_property("x", &TestBaseInstance::x),
            def_property("y", &TestBaseInstance::y)
        );
    }

    INSTANCE_HEADER_SOURCE
};

class TestDerivedInstance : public TestBaseInstance {
public:
    int x2;
    int z;
private:
    static InstanceType __buildType() {
        return make_instance_type<TestDerivedInstance, TestBaseInstance>(
            "TestDerivedInstance",

            def_property("x", &TestDerivedInstance::x2),
            def_property("z", &TestDerivedInstance::z)
        );
    }

    INSTANCE_HEADER_SOURCE
};

TEST_CASE("Member inheritance") {
    SECTION("Constructor") {
        auto apex = Instance::Type();
        auto base = TestBaseInstance::Type();
        auto derived = TestDerivedInstance::Type();

        REQUIRE(!apex.constructor.has_value());
        REQUIRE(base.constructor.has_value());
        REQUIRE(derived.constructor.has_value());
    }

    SECTION("Property inheritance") {
        auto type = TestDerivedInstance::Type();
        REQUIRE(type.properties.size() == 3);

        std::shared_ptr<TestDerivedInstance> foo = std::make_shared<TestDerivedInstance>();
        foo->x = 1;
        foo->y = 2;
        foo->z = 3;
        foo->x2 = 4;

        REQUIRE(type.properties["x"].getter(foo).get<int>() == 4);
        REQUIRE(type.properties["y"].getter(foo).get<int>() == 2);
        REQUIRE(type.properties["z"].getter(foo).get<int>() == 3);
    }
}