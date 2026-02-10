#include <catch2/catch_test_macros.hpp>
#include "datatypes/ref.h"
#include "enum/part.h"
#include "objects/part/part.h"
#include "objectmodel/datatypes.h"

// Tests for interim class

TEST_CASE("Type_meta_of") {
    REQUIRE(type_meta_of<std::shared_ptr<Part>>().descriptor == &InstanceRef::TYPE);
    REQUIRE(type_meta_of<std::shared_ptr<Part>>().instType == &Part::TYPE);

    REQUIRE(type_meta_of<PartType>().descriptor == &EnumItem::TYPE);
    REQUIRE(type_meta_of<PartType>().enum_ == &EnumType::PartType);
}