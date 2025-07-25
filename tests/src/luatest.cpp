#include "testutil.h"
#include "testutillua.h"

#include <memory>


void test_output(DATAMODEL_REF m) {
    ASSERT_EQ("INFO: Hello, world!\n", luaEvalOut(m, "print('Hello, world!')"));
    // ASSERT_EQ("WARN: Some warning here.\n", luaEvalOut(m, "warn('Some warning here.')"));
    // ASSERT_EQ("ERROR: An error!.\n", luaEvalOut(m, "error('An error!')"));
}

int main() {
    auto m = DataModel::New();
    m->Init(true);

    test_output(m);

    return TEST_STATUS;
}