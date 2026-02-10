#include <catch2/catch_test_macros.hpp>
#include "objects/base/instance.h"
#include "objectmodel/type.h"

class TestInstance : public Instance {
public:

private:
    static const InstanceType2 __buildType() {
        return make_instance_type<TestInstance>("TestInstance");
    }

    INSTANCE_HEADER_SOURCE
};

TEST_CASE("Introspection") {
    SECTION ("Name") {
        REQUIRE(TestInstance::Type().className == "TestInstance");
    }
}