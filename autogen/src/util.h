#pragma once

#include <clang-c/Index.h>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <filesystem>

typedef std::function<CXChildVisitResult(CXCursor cursor, CXCursor parent)> X_CXCursorVisitor;

unsigned x_clang_visitChildren(CXCursor parent, X_CXCursorVisitor visitor);

std::string x_clang_toString(CXString string);

// Very simple parser
// Example format:
//  name="Hello!", world=Test, read_only
// Result:
//  "name": "Hello!", "world": "Test", "read_only": ""
std::map<std::string, std::string> parseAnnotationString(std::string src);

std::optional<std::string> findAnnotation(CXCursor cur, std::string annotationName);

std::string string_of(std::filesystem::path path);