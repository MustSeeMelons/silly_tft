#include "esp_log.h"
#include "include/scene.h"
#include "include/tft.h"
#include "include/utils.h"

extern uint16_t curr_display_height;
extern uint16_t curr_display_width;

// clang-format off
vector2_t points[4] = {
    {.x = 50, .y = 50},
    {.x = 150, .y = 50},
    {.x = 150, .y = 150},
    {.x = 50, .y = 150}
};
// clang-format on

scene_object_t *lines[4] = {NULL, NULL, NULL, NULL};

void cube_init(void *ctx) {
    for (size_t i = 0; i < 4; i++) {
        size_t next_index = (i + 1) % 4;

        lines[i] = tft_add_line(
            points[i].x, points[i].y, points[next_index].x, points[next_index].y, get_color565(255, 255, 255), 1);
    }
}

void cube_update(void *ctx) {
}

scene_t cube_scene = {.scene_init = cube_init, .scene_update = cube_update};