#include <catch2/catch_test_macros.hpp>
#include "objectmodel/macro.h"
#include "objectmodel/property.h"
#include "objectmodel/type.h"

class TestInstance2 : public Instance2 {
public:
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;

private:
    static InstanceType2 __buildType() {
        return make_instance_type<TestInstance2>(
            "TestInstance2",

            def_property("a", &TestInstance2::a),
            set_property_category("data"),
            def_property("b", &TestInstance2::b),
            def_property("c", &TestInstance2::c),
            set_property_category("appearance"),
            def_property("d", &TestInstance2::d),
            def_property("e", &TestInstance2::e),
            def_property("f", &TestInstance2::f)
        );
    }

    INSTANCE_HEADER_SOURCE
};

TEST_CASE ("Property categories") {
    auto type = TestInstance2::Type();
    REQUIRE(type.properties["a"].category == "");
    REQUIRE(type.properties["b"].category == "data");
    REQUIRE(type.properties["c"].category == "data");
    REQUIRE(type.properties["d"].category == "appearance");
    REQUIRE(type.properties["e"].category == "appearance");
    REQUIRE(type.properties["f"].category == "appearance");
}