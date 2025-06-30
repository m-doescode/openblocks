#pragma once

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wnullability-extension"
#define nullable _Nullable
#define notnull _Nonnull
#elif defined(__GNUC__)
#define nullable
#define notnull __attribute__((nonnull))
#else
#define nullable
#define notnull
#endif