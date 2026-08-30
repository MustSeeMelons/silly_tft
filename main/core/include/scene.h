#ifndef SCENE_H_
#define SCENE_H_

#include "core/include/display.h"
#include "core/include/gfx.h"

typedef struct {
    void (*scene_init)(display_config_t *display_config);
    void (*scene_update)(void *ctx); // NOTE: This should have a delta param for ultimate smoooothness
} scene_t;

#endif