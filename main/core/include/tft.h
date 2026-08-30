#ifndef TFT_H_
#define TFT_H_

#include "core/include/defs.h"
#include "core/include/display.h"
#include "core/include/gfx.h"
#include "core/include/math_functions.h"

typedef struct {
    display_config_t *display_config;
    tft_rotation_t rotation;
} tft_init_config_t;

gfx_t *tft_init(tft_init_config_t config);

void tft_reset();

void tft_clear();

#endif