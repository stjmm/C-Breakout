#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <linmath.h>

#define ERROR_EXIT(...) { fprintf(stderr, __VA_ARGS__); exit(1); }
#define ERROR_RETURN(R, ...)  { fprintf(stderr, __VA_ARGS__); return R; }

static inline float vec2_dot(vec2 a, vec2 b) {
    return a[0] * b[0] + a[1] * b[1];
}
