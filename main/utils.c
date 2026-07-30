#include "include/utils.h"

uint16_t get_color565(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t red = (r & 0xf8) << 8;
    uint16_t green = (g & 0xfc) << 3;
    uint16_t blue = (b & 0xf8) >> 3;

    return red | green | blue;
}

int16_t get_min_x(vector2_t vertices[3]) {
    int16_t min_x = vertices[0].x;

    for (int i = 1; i < 3; i++) {
        if (vertices[i].x < min_x) {
            min_x = vertices[i].x;
        }
    }

    return min_x;
}

int16_t get_max_x(vector2_t vertices[3]) {
    int16_t max_x = vertices[0].x;

    for (int i = 1; i < 3; i++) {
        if (vertices[i].x > max_x) {
            max_x = vertices[i].x;
        }
    }

    return max_x;
}

int16_t get_min_y(vector2_t vertices[3]) {
    int16_t min_y = vertices[0].y;

    for (int i = 1; i < 3; i++) {
        if (vertices[i].y < min_y) {
            min_y = vertices[i].y;
        }
    }

    return min_y;
}

int16_t get_max_y(vector2_t vertices[3]) {
    int16_t max_y = vertices[0].y;

    for (int i = 1; i < 3; i++) {
        if (vertices[i].y > max_y) {
            max_y = vertices[i].y;
        }
    }

    return max_y;
}

void swap_int16(int16_t *one, int16_t *other) {
    int16_t temp = *one;
    *one = *other;
    *other = temp;
}