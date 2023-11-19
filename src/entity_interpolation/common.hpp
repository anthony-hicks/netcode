#pragma once

#include <chrono>

#define DECLARE_COPY(T, keyword)                                                    \
    T(T const&) = keyword;                                                          \
    T& operator=(T const&) = keyword

#define DEFAULT_COPY(T) DECLARE_COPY(T, default)
#define DISABLE_COPY(T) DECLARE_COPY(T, delete)

#define DECLARE_MOVE(T, keyword)                                                    \
    T(T&&) = keyword;                                                               \
    T& operator=(T&&) = keyword

#define DEFAULT_MOVE(T) DECLARE_MOVE(T, default)
#define DISABLE_MOVE(T) DECLARE_MOVE(T, delete)

#define DECLARE_CTOR(T, keyword) \
    T() = keyword

#define DEFAULT_CTOR(T) DECLARE_CTOR(T, default)
#define DISABLE_CTOR(T) DECLARE_CTOR(T, delete)

using milliseconds_d = std::chrono::duration<double, std::milli>;
