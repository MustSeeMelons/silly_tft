#include "esp_log.h"
#include "include/scene.h"
#include "include/tft.h"
#include "include/utils.h"

extern uint16_t curr_display_height;
extern uint16_t curr_display_width;

extern gfx_t *active_gfx;

// clang-format off
// The original cube vertexes
point_vector_t points[8] = {
    {.x = 120, .y = 80, .z = -40},
    {.x = 200, .y = 80, .z = -40},
    {.x = 200, .y = 160, .z = -40},
    {.x = 120, .y = 160, .z = -40},
    {.x = 120, .y = 80, .z = 40},
    {.x = 200, .y = 80, .z = 40},
    {.x = 200, .y = 160, .z = 40},
    {.x = 120, .y = 160, .z = 40}
};

// Working copy of the vertexes for a frame
point_vector_t screen_points[8];
// clang-format on

// Rotation pivot point (center of the cube)
point_vector_t pivot = {.x = 160, .y = 120, .z = 0};

float angle = 0;
float camera_distance = 150.0f;
float focal_length = 150.0f;
uint8_t line_idx = 0;

scene_object_t *lines[12] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
scene_object_t *circles[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static void draw_line(uint8_t i, uint8_t j) {
    point_vector_t a = screen_points[i];
    point_vector_t b = screen_points[j];

    lines[line_idx++] = active_gfx->gfx_add_line(a.x, a.y, b.x, b.y, get_color565(255, 255, 255), 1);
}

static void update_line(uint8_t i, uint8_t j) {
    point_vector_t a = screen_points[i];
    point_vector_t b = screen_points[j];

    scene_object_t *line = lines[line_idx++];

    line->x = a.x;
    line->y = a.y;
    line->line.x_end = b.x;
    line->line.y_end = b.y;
}

void cube_init(void *ctx) {
    for (size_t i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
        point_vector_t transformed = points[i];

        perspective_projection(&transformed, &pivot, &screen_points[i], focal_length, camera_distance);

        circles[i] = active_gfx->gfx_add_circle(screen_points[i].x, screen_points[i].y, 5, get_color565(255, 255, 255));
    }

    for (size_t i = 0; i < 4; i++) {
        draw_line(i, (i + 1) % 4);
        draw_line(i + 4, ((i + 1) % 4) + 4);
        draw_line(i, i + 4);
    }

    active_gfx->gfx_global_update();
}

void cube_update(void *ctx) {
    angle += 0.05f;
    line_idx = 0;

    for (size_t i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
        point_vector_t transformed = points[i];

        rotate_point_x(&transformed, &pivot, angle);
        rotate_point_y(&transformed, &pivot, angle);
        rotate_point_z(&transformed, &pivot, angle);

        perspective_projection(&transformed, &pivot, &screen_points[i], focal_length, camera_distance);

        circles[i]->x = screen_points[i].x;
        circles[i]->y = screen_points[i].y;
    }

    for (size_t i = 0; i < 4; i++) {
        update_line(i, (i + 1) % 4);
        update_line(i + 4, ((i + 1) % 4) + 4);
        update_line(i, i + 4);
    }

    active_gfx->gfx_global_update();
}

scene_t cube_scene = {.scene_init = cube_init, .scene_update = cube_update};