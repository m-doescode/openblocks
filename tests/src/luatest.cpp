#include "testutil.h"

#include "logger.h"
#include "objects/datamodel.h"
#include "objects/script.h"
#include <memory>
#include <sstream>

std::string luaEval(DATAMODEL_REF m, std::string source) {
    std::stringstream out;
    Logger::initTest(&out);

    auto s = Script::New();
    m->AddChild(s);
    s->source = source;
    s->Run();

    return out.str();
}

void test_output(DATAMODEL_REF m) {
    ASSERT_EQ("INFO: Hello, world!\n", luaEval(m, "print('Hello, world!')"));
    // ASSERT_EQ("WARN: Some warning here.\n", luaEval(m, "warn('Some warning here.')"));
    // ASSERT_EQ("ERROR: An error!.\n", luaEval(m, "error('An error!')"));
}

int main() {
    auto m = DataModel::New();
    m->Init(true);

    test_output(m);

    return TEST_STATUS;
}