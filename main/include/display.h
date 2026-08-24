#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "include/definitions.h"
#include "stdint.h"

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t dataCount; // Low stores number, high stores flag for delay. 0xFF as end marker
} lcd_init_cmd_t;

// Defines display specific functions for init and set window, as well as width and height getters
typedef struct {
    lcd_init_cmd_t *(*display_init_commands)(tft_rotation_t rotation);
    void (*set_window)(int16_t x_start, int16_t y_start, int16_t x_end, int16_t y_end);
    uint16_t (*get_width)();
    uint16_t (*get_height)();
} display_config_t;

#endif