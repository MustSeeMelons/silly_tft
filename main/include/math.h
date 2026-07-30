#ifndef MATH_H_
#define MATH_H_

#include "stdint.h"

typedef struct {
    int16_t x;
    int16_t y;
} vector2_t;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} vector3_t;

float lerp_f(float start, float end, float blend_factor);

int16_t lerp_i(int16_t start, int16_t end, float blend_factor);

float get_blend_factor(int16_t value, int16_t value_start, int16_t value_end);

int32_t edge_function(vector2_t a, vector2_t b, vector2_t c);

#endif