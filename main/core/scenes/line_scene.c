#include "core/include/scene.h"
#include "core/include/tft.h"
#include "core/include/utils.h"

extern gfx_t *active_gfx;

scene_object_t *line = NULL;

void lines_init(display_config_t *display_config) {
    line = active_gfx->gfx_add_line(
        0, 0, display_config->get_width(), display_config->get_height(), get_color565(0, 255, 0), 1);
}

void lines_update(void *ctx) {
}

scene_t line_scene = {.scene_init = lines_init, .scene_update = lines_update};