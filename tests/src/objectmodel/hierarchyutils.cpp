#include "objects/datamodel.h"
#include "objects/model.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>

#define DECL_CLASS(_ClassName, _Parent) \
class _ClassName : public Instance { \
    static InstanceType __buildType() { \
        return make_instance_type<_ClassName, _Parent>(#_ClassName); \
    } \
\
    INSTANCE_HEADER_SOURCE \
}

static inline std::shared_ptr<Model> append_model(std::shared_ptr<Instance> parent, std::string name) {
    auto model = Model::New();
    model->name = name;
    model->SetParent(parent);
    return model;
}

template <typename T> static inline std::shared_ptr<T> append_inst(std::shared_ptr<Instance> parent, std::string name) {
    auto model = new_instance<T>();
    model->name = name;
    model->SetParent(parent);
    return model;
}

DECL_CLASS(HU_TI1, Instance);
DECL_CLASS(HU_TI1_2, HU_TI1);
DECL_CLASS(HU_TI2, Instance);
DECL_CLASS(HU_TI3, Instance);
DECL_CLASS(HU_TI4, Instance);

TEST_CASE("Hierarchy utilities") {
    auto root = DataModel::New();
    root->Init(true);
    
    SECTION("GetFullName") {
        auto m = append_model(root, "foo"); 
        m = append_model(m, "bar"); 
        m = append_model(m, "baz");
        REQUIRE(m->GetFullName() == "foo.bar.baz");
    }

    SECTION("FindFirstChild") {
        auto m = append_model(root, "");
        auto m1_2 = append_inst<HU_TI1_2>(m, "D");
        auto m1 = append_inst<HU_TI1>(m, "A");
        auto m2 = append_inst<HU_TI2>(m, "B");
        auto m3 = append_inst<HU_TI3>(m, "C");

        REQUIRE(m->FindFirstChild("B") == m2);
        REQUIRE(m->FindFirstChildOfClass("HU_TI1") == m1);
        REQUIRE(m->FindFirstChildWhichIsA("HU_TI1") == m1_2);
    }

    SECTION("FindFirstAncestor") {
        auto m = append_model(root, "");
        auto m1 = append_inst<HU_TI1>(m, "A");
        auto m1_2 = append_inst<HU_TI1_2>(m1, "B");
        auto m3 = append_inst<HU_TI3>(m1_2, "C");
        auto m4 = append_inst<HU_TI2>(m3, "D");

        REQUIRE(m4->FindFirstAncestor("D") == nullptr);
        REQUIRE(m4->FindFirstAncestor("C") == m3);
        REQUIRE(m4->FindFirstAncestorOfClass("HU_TI1") == m1);
        REQUIRE(m4->FindFirstAncestorWhichIsA("HU_TI1") == m1_2);
    }

    SECTION("GetChildren/GetDescendants") {
        auto m = append_model(root, "");
        auto m1 = append_model(m, "");
        auto m2 = append_model(m, "");
        auto m3 = append_model(m2, "");
        auto m4 = append_model(m2, "");
        auto m5 = append_model(m, "");
        auto m6 = append_model(m5, "");
        auto m7 = append_model(m5, "");

        REQUIRE(m->GetChildren() == std::vector<std::shared_ptr<Instance>> { m1, m2, m5 });
        REQUIRE(m->GetDescendants() == std::vector<std::shared_ptr<Instance>> { m1, m2, m3, m4, m5, m6, m7 });
    }
}