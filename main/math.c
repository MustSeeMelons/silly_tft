#include "include/math.h"

float lerp_f(float start, float end, float blend_factor) {
    return start + blend_factor * (end - start);
}

int16_t lerp_i(int16_t start, int16_t end, float blend_factor) {
    return (int16_t)(start + blend_factor * (end - start));
}

float get_blend_factor(int16_t value, int16_t value_start, int16_t value_end) {
    if (value_start == value_end) {
        return 0.0f;
    }

    if (value_start < value_end) {
        if (value <= value_start) {
            return 0.0f;
        }

        if (value >= value_end) {
            return 1.0f;
        }
    } else {
        if (value >= value_start) {
            return 0.0f;
        }

        if (value <= value_end) {
            return 1.0f;
        }
    }

    return (float)(value - value_start) / (value_end - value_start);
}

int32_t edge_function(vector2_t a, vector2_t b, vector2_t c) {
    return (int32_t)(b.x - a.x) * (c.y - a.y) - (int32_t)(b.y - a.y) * (c.x - a.x);
}
