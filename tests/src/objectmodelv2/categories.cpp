#include <catch2/catch_test_macros.hpp>
#include "objectmodel/macro.h"
#include "objectmodel/property.h"
#include "objectmodel/type.h"
#include "objects/base/instance.h"

// Important: Make sure that the class name is unique in each test file
class TestInstance3 : public Instance {
public:
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;

private:
    static InstanceType __buildType() {
        return make_instance_type<TestInstance3>(
            "TestInstance3",

            def_property("a", &TestInstance3::a),
            set_property_category("data"),
            def_property("b", &TestInstance3::b),
            def_property("c", &TestInstance3::c),
            set_property_category("appearance"),
            def_property("d", &TestInstance3::d),
            def_property("e", &TestInstance3::e),
            def_property("f", &TestInstance3::f)
        );
    }

    INSTANCE_HEADER_SOURCE
};

TEST_CASE("Property categories") {
    auto type = TestInstance3::Type();
    REQUIRE(type.properties["a"].category == "");
    REQUIRE(type.properties["b"].category == "data");
    REQUIRE(type.properties["c"].category == "data");
    REQUIRE(type.properties["d"].category == "appearance");
    REQUIRE(type.properties["e"].category == "appearance");
    REQUIRE(type.properties["f"].category == "appearance");
}