#ifndef TFT_H_
#define TFT_H_

#include "driver/spi_master.h"
#include "esp_err.h"
#include "include/gfx.h"
#include "include/math_functions.h"

// Save on memory at the cost of max FPS.

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t dataCount; // Low stores number, high stores flag for delay. 0xFF as end marker
} lcd_init_cmd_t;

typedef enum
{
    TFT_Portrait = 0x48,         // 0 Degrees
    TFT_Landscape = 0x28,        // 90 Degrees
    TFT_Portrait_Flipped = 0x88, // 180 Degrees
    TFT_Landscape_Flipped = 0xE8 // 270 Degrees
} tft_rotation_t;

typedef struct {
    tft_rotation_t rotation;
} tft_init_config_t;

gfx_t *tft_init(tft_init_config_t config);

void tft_reset();

void tft_patch_init_commands(tft_rotation_t rotation);

void tft_clear();

void tft_send_command(uint8_t cmd);

void tft_send_data_byte(uint8_t data);

void tft_send_data(uint8_t *data, size_t len);

void tft_set_window(int16_t x_start, int16_t y_start, int16_t x_end, int16_t y_end);

#endif