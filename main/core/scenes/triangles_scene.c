#include "core/include/scene.h"
#include "core/include/tft.h"
#include "core/include/utils.h"

extern gfx_t *active_gfx;

scene_object_t *triangle = NULL;

void triangles_init(display_config_t *display_config) {
    // clang-format off
    triangle = active_gfx->gfx_add_triangle((vector2_t[3]){
        {.x = 50, .y = 10},
        {.x = 10, .y = 90 },
        {.x = 100, .y = 200}
    }, get_color565(255, 0, 0));
    // clang-format on
}

void triangles_update(void *ctx) {
}

scene_t triangles_scene = {.scene_init = triangles_init, .scene_update = triangles_update};