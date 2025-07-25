#pragma once

// https://bastian.rieck.me/blog/2017/simple_unit_tests/

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>

#ifdef __FUNCTION__
#define __FUNC_NAME __FUNCTION__
#else
#define __FUNC_NAME __func__
#endif

#define ASSERT(x, msg) __assert((x), __FILE__, __LINE__, __FUNC_NAME, msg)
#define ASSERT_EQ(x, y) __assert_eq((x) == (y), __FILE__, __LINE__, __FUNC_NAME, #x, (y))
// #define ASSERT_EQSTR(x, y) ASSERT(strcmp(x, y) == 0, #x " != " #y)
#define ASSERT_EQSTR(x, y) ASSERT_EQ(x, y)

#define DATAMODEL_REF std::shared_ptr<DataModel>

#define TU_TIME_EXPOSE_TEST
#define TT_ADVANCETIME(secs) tu_set_override(tu_clock_micros() + (secs) * 1'000'000);

#include <cstdio>
#include <cstring>

int TEST_STATUS = 0;

inline void __assert(bool cond, std::string file, int line, std::string func, std::string message) {
    if (cond) return;
    fprintf(stderr, "ASSERT FAILED : %s:%d : %s : '%s'\n", file.c_str(), line, func.c_str(), message.c_str());
    TEST_STATUS = 1;
}

template <typename T>
inline std::string quote(T value) {
    return std::to_string(value);
}

template <>
std::string quote<std::string>(std::string value) {
    std::stringstream ss;
    ss << std::quoted(value);
    std::string newstr = ss.str();

    newstr = std::regex_replace(newstr, std::regex("\n"), "\\n");
    return newstr;
}

template <>
inline std::string quote<const char*>(const char* value) {
    return quote<std::string>(value);
}

template <>
inline std::string quote<char*>(char* value) {
    return quote<std::string>(value);
}

template <typename T>
void __assert_eq(bool cond, std::string file, int line, std::string func, std::string model, T value) {
    if (cond) return;
    std::string message = model + " != " + quote(value);
    fprintf(stderr, "ASSERT FAILED : %s:%d : %s : '%s'\n", file.c_str(), line, func.c_str(), message.c_str());
    TEST_STATUS = 1;
}