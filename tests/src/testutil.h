#pragma once

// https://bastian.rieck.me/blog/2017/simple_unit_tests/

#ifdef __FUNCTION__
#define ASSERT(x, msg) if (!(x)) { fprintf(stderr, "ASSERT FAILED : %s:%d : %s : '%s'\n", __FILE__, __LINE__, __FUNCTION__, msg); exit(1); TEST_STATUS = 1; }
#else
#define ASSERT(x, msg) if (!(x)) { fprintf(stderr, "ASSERT FAILED : %s:%d : ?? : '%s'\n", __FILE__, __LINE__, msg); TEST_STATUS = 1; }
#endif

#define ASSERT_EQ(x, y) ASSERT(x == y, #x " != " #y)
#define ASSERT_EQSTR(x, y) ASSERT(strcmp(x, y) == 0, #x " != " #y)

#define DATAMODEL_REF std::shared_ptr<DataModel>

#include <cstdio>
#include <cstring>

int TEST_STATUS = 0;