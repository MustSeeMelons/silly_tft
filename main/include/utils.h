#ifndef UTILS_H_
#define UTILS_H_

#include "include/math.h"
#include "include/tft.h"
#include "stdint.h"

uint16_t get_color565(uint8_t r, uint8_t g, uint8_t b);

int16_t get_min_x(vector2_t vertices[3]);

int16_t get_min_y(vector2_t vertices[3]);

int16_t get_max_x(vector2_t vertices[3]);

int16_t get_max_y(vector2_t vertices[3]);

void swap_int16(int16_t *one, int16_t *other);

#endif