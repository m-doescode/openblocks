#pragma once

#include <string>

class StringData {
    std::string wrapped;
public:
    StringData(std::string);
    operator std::string();
};

class IntData {
    int wrapped;
public:
    IntData(int);
    operator int();
};

class FloatData {
    float wrapped;
public:
    FloatData(float);
    operator float();
};