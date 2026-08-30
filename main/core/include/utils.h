#ifndef UTILS_H_
#define UTILS_H_

#include "core/include/gfx.h"
#include "core/include/math_functions.h"
#include "core/include/tft.h"
#include "stdint.h"

uint16_t get_color565(uint8_t r, uint8_t g, uint8_t b);

int16_t get_min_x(vector2_t vertices[3]);

int16_t get_min_y(vector2_t vertices[3]);

int16_t get_max_x(vector2_t vertices[3]);

int16_t get_max_y(vector2_t vertices[3]);

void app_do_fps(scene_object_t *text);

#endif