#pragma once

#ifdef __clang__
#pragma clang diagnostic ignored "-Wnullability-extension"
#define nullable _Nullable
#define notnull _Nonnull
#else
#define nullable
#define notnull
#endif