#include "util.h"
#include <clang-c/CXString.h>
#include <optional>

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

std::map<std::string, std::string> parseAnnotationString(std::string src) {
    std::map<std::string, std::string> result;

    std::string currentIdent = "";
    std::string currentValue = "";
    int stage = 0;
    bool quoted = false;

    int i = 0;
    for (; i < src.length(); i++) {
        if (src[i] == ' ' && (stage != 2 || !quoted)) continue; // Ignore spaces if not in stage 2 and quoted
        if (src[i] == ',' && stage == 0) continue; // Let empty commas slip by
        if (stage < 2 && (src[i] >= 'a' && src[i] <= 'z' || src[i] >= 'A' && src[i] <= 'Z' || src[i] >= '0' && src[i] <= '9' || src[i] == '_')) {
            currentIdent += src[i];
            stage = 1;
            continue;
        }
        if (stage == 1 && src[i] == '=') { // What follows is a value
            stage = 2;
            continue;
        }
        if (stage == 1 && src[i] == ',') { // Value-less key
            stage = 0;
            result[currentIdent] = "";
            currentIdent = "";
            continue;
        }
        if (stage == 2 && quoted && src[i] == '"') { // Close a quoted string
            quoted = false;
            continue;
        }
        if (stage == 2 && !quoted && src[i] == '"') { // Start a quoted string
            quoted = true;
            continue;
        }
        if (stage == 2 && !quoted && (src[i] == ' ' || src[i] == ',')) { // Terminate the string
            stage = 0;
            result[currentIdent] = currentValue;
            currentIdent = "";
            currentValue = "";
            continue;
        }
        if (stage == 2) { // Otherwise if in stage 2, simply add the character
            currentValue += src[i];
            continue;
        }
        fprintf(stderr, "Unexpected symbol: %c at index %d\n", src[i], i);
        fprintf(stderr, "\t%s\n", src.c_str());
        fprintf(stderr, "\t%s^\n", i > 0 ? std::string(i, '~').c_str() : "");
        abort();
    }

    // Add the remaining value
    if (stage == 1) {
        result[currentIdent] = "";
    } else if (stage == 2) {
        result[currentIdent] = currentValue;
    }

    return result;
}

std::optional<std::string> findAnnotation(CXCursor cur, std::string annotationName) {
    std::optional<std::string> ret = std::nullopt;

    x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
        CXCursorKind kind = clang_getCursorKind(cur);
        if (kind != CXCursor_AnnotateAttr) return CXChildVisit_Continue;

        std::string annString = x_clang_toString(clang_getCursorDisplayName(cur));
        if (annString != annotationName) return CXChildVisit_Continue;

        // Look inside for a StringLiteral

        x_clang_visitChildren(cur, [&](CXCursor cur, CXCursor parent) {
            CXCursorKind kind = clang_getCursorKind(cur);
            // if (kind != CXCursor_StringLiteral) return CXChildVisit_Recurse;
            // String literals cannot be parsed as CXCursor_StringLiteral. I don't know why.
            if (kind != CXCursor_UnexposedExpr) return CXChildVisit_Recurse;

            // https://stackoverflow.com/a/63859988/16255372
            auto res = clang_Cursor_Evaluate(cur);
            ret = clang_EvalResult_getAsStr(res);
            clang_EvalResult_dispose(res);

            return CXChildVisit_Break;
        });

        return CXChildVisit_Break;
    });
    return ret;
}