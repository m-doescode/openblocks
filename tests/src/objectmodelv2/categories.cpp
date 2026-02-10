#include <catch2/catch_test_macros.hpp>
#include "objectmodel/macro.h"
#include "objectmodel/property.h"
#include "objectmodel/type.h"
#include "objects/base/instance.h"

class TestInstance : public Instance {
public:
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;

private:
    static InstanceType __buildType() {
        return make_instance_type<TestInstance>(
            "TestInstance",

            def_property("a", &TestInstance::a),
            set_property_category("data"),
            def_property("b", &TestInstance::b),
            def_property("c", &TestInstance::c),
            set_property_category("appearance"),
            def_property("d", &TestInstance::d),
            def_property("e", &TestInstance::e),
            def_property("f", &TestInstance::f)
        );
    }

    INSTANCE_HEADER_SOURCE
};

TEST_CASE("Property categories") {
    auto type = TestInstance::Type();
    REQUIRE(type.properties["a"].category == "");
    REQUIRE(type.properties["b"].category == "data");
    REQUIRE(type.properties["c"].category == "data");
    REQUIRE(type.properties["d"].category == "appearance");
    REQUIRE(type.properties["e"].category == "appearance");
    REQUIRE(type.properties["f"].category == "appearance");
}