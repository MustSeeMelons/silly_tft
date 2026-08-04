#ifndef SCENE_H_
#define SCENE_H_

#include "include/gfx.h"

typedef struct {
    void (*scene_init)(void *ctx);
    void (*scene_update)(void *ctx); // NOTE: This should have a delta param for ultimate smoooothness
} scene_t;

#endif