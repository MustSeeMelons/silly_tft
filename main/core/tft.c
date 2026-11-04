#include "core/include/tft.h"
#include "core/include/defs.h"
#include "core/include/display.h"
#include "core/include/gfx.h"
#include "core/include/platform.h"
#include "core/include/spi_display.h"
#include "core/include/utils.h"
#include "math.h"
#include <string.h>

extern spi_display_t *spi_display;

extern gfx_t scanline_gfx;

static display_config_t *config = NULL;

gfx_t *tft_init(tft_init_config_t init_config) {
    platform_set_led(false);

    config = init_config.display_config;

    // Apply rotation to init commands
    lcd_init_cmd_t *lcd_init_cmds = config->display_init_commands(init_config.rotation);

    uint8_t init_cmd_index = 0;

    // Power ON sequence
    while (lcd_init_cmds[init_cmd_index].dataCount != 0xff) {
        spi_display->spi_send_command(lcd_init_cmds[init_cmd_index].cmd);

        uint8_t data_len = lcd_init_cmds[init_cmd_index].dataCount & 0x1F; // 5 bits have length
        if (data_len > 0) {
            spi_display->spi_send_data(lcd_init_cmds[init_cmd_index].data, data_len);
        }

        if (lcd_init_cmds[init_cmd_index].dataCount & 0x80) { // High bit signals delay
            delay_ms(100);
        }

        init_cmd_index++;
    }

    platform_set_led(true);

    scanline_gfx.gfx_init(config);

    return &scanline_gfx;
}

void tft_reset() {
    scanline_gfx.gfx_reset();
    tft_clear();
}

void tft_clear() {
    config->set_window(0, 0, config->get_width() - 1, config->get_height() - 1);

    uint16_t color = 0x0000;

    // One full row of pixels
    uint8_t line[config->get_width() * 2];

    // XXX Line by line is inneficent
    for (int x = 0; x < config->get_width(); x++) {
        line[x * 2] = color >> 8;
        line[x * 2 + 1] = color & 0xFF;
    }

    for (int y = 0; y < config->get_height(); y++) {
        spi_display->spi_send_data(line, sizeof(line));
    }
}
