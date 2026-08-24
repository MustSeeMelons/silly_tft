#ifndef TFT_H_
#define TFT_H_

#include "driver/spi_master.h"
#include "esp_err.h"
#include "include/definitions.h"
#include "include/display.h"
#include "include/gfx.h"
#include "include/math_functions.h"

#define TFT_LED 23 // Backlight

typedef struct {
    display_config_t *display_config;
    tft_rotation_t rotation;
} tft_init_config_t;

gfx_t *tft_init(tft_init_config_t config);

void tft_reset();

void tft_clear();

#endif