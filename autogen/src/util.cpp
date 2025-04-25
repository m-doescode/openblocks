#include "util.h"
#include <clang-c/CXString.h>

static CXChildVisitResult _visitorFunc(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    X_CXCursorVisitor* func = (X_CXCursorVisitor*)client_data;
    return (*func)(cursor, parent);
}

unsigned x_clang_visitChildren(CXCursor parent, X_CXCursorVisitor visitor) {
    return clang_visitChildren(parent, _visitorFunc, &visitor);
}

std::string x_clang_toString(CXString string) {
    std::string str(clang_getCString(string));
    clang_disposeString(string);
    return str;
}