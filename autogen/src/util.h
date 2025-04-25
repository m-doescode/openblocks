#pragma once

#include <clang-c/Index.h>
#include <functional>
#include <string>

typedef std::function<CXChildVisitResult(CXCursor cursor, CXCursor parent)> X_CXCursorVisitor;

unsigned x_clang_visitChildren(CXCursor parent, X_CXCursorVisitor visitor);

std::string x_clang_toString(CXString string);