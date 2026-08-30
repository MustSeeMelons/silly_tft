#ifndef GFX_H_
#define GFX_H_

#include "core/include/defs.h"
#include "core/include/display.h"
#include "core/include/font.h"
#include "core/include/math_functions.h"
#include "stdint.h"
#include <stdbool.h>
#include <string.h>

#define GFX_SCENE_OBJECT_MAX 32

typedef enum
{
    OBJECT_RECTANGLE,
    OBJECT_TEXT,
    OBJECT_CIRCLE,
    OBJECT_TRIANGLE,
    OBJECT_LINE
} scene_object_type_t;

typedef struct {
    int32_t step_x;
    int32_t step_y;
} edge_step_t;

typedef struct {
    uint8_t id;
    scene_object_type_t type;
    int16_t x;
    int16_t y;
    union {
        struct {
            uint16_t width;
            uint16_t height;
            uint16_t color;
        } rectangle;
        struct {
            char *text;
            uint16_t font_size;
            uint16_t width;
            uint16_t color;
        } text;
        struct {
            uint16_t radius;
            uint16_t color;
        } circle;
        struct {
            vector2_t vertices[3];
            int16_t min_x;
            int16_t min_y;
            int16_t max_x;
            int16_t max_y;
            edge_step_t ab;
            edge_step_t bc;
            edge_step_t ca;
            uint16_t color;
        } triangle;
        struct {
            int16_t x_end;
            int16_t y_end;
            uint16_t color;
            uint8_t stroke;
        } line;
    };
} scene_object_t;

typedef struct {
    platform_err_t error;
    void (*gfx_init)(display_config_t *config);
    void (*gfx_reset)(void);
    scene_object_t *(*gfx_add_rectangle)(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color);
    scene_object_t *(*gfx_add_text)(int16_t x, int16_t y, char *text, uint16_t font_size, uint16_t color);
    scene_object_t *(*gfx_add_circle)(int16_t x, int16_t y, uint16_t radius, uint16_t color);
    scene_object_t *(*gfx_add_triangle)(vector2_t vertices[3], uint16_t color);
    scene_object_t *(*gfx_add_line)(
        int16_t x_start, int16_t y_start, int16_t x_end, int16_t y_end, uint16_t color, uint8_t stroke);
    // To perform some calculations after a prop change, required by triangles
    void (*gfx_update_scene_object)(scene_object_t *obj);
    // Might need to be called when moving objects around or changing size
    void (*gfx_global_update)(void);
    void (*gfx_render)(void);
} gfx_t;

#endif