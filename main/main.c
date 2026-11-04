#include "core/include/defs.h"
#include "core/include/platform.h"
#include "core/include/scene.h"
#include "core/include/spi_display.h"
#include "core/include/tft.h"
#include "core/include/utils.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

#define TFT_LED 23 // Backlight

void delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

uint64_t now_ms(void) {
    return esp_timer_get_time() / 1000;
}

void platform_log(platform_log_level_t level, const char *tag, const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    switch (level) {
    case PLATFORM_LOG_LEVEL_WARN:
        esp_log_write(level, tag, LOG_FORMAT(W, "%s"), esp_log_timestamp(), tag, buf);
        break;
    case PLATFORM_LOG_LEVEL_INFO:
        esp_log_write(level, tag, LOG_FORMAT(I, "%s"), esp_log_timestamp(), tag, buf);
        break;
    case PLATFORM_LOG_LEVEL_DEBUG:
        esp_log_write(level, tag, LOG_FORMAT(D, "%s"), esp_log_timestamp(), tag, buf);
        break;
    case PLATFORM_LOG_LEVEL_ERROR:
    case PLATFORM_LOG_LEVEL_NONE:
    default:
        esp_log_write(level, tag, LOG_FORMAT(E, "%s"), esp_log_timestamp(), tag, buf);
        break;
    }
}

void platform_set_led(bool on) {
    gpio_set_level(TFT_LED, on ? 1 : 0);
}

static const char *TAG = "Main";

extern scene_t cube_scene;

// The config to use
// extern display_config_t ili9341_display_config;
extern display_config_t gc9a01_display_config;

extern spi_display_t esp_spi;
spi_display_t *spi_display = &esp_spi;

gfx_t *active_gfx;

void app_main(void) {
    platform_err_t err = esp_spi.spi_init();

    if (err != PLATFORM_OK) {
        platform_log(PLATFORM_LOG_LEVEL_ERROR, TAG, "Failed to initialize SPI: %d", err);
        return;
    }

    // clang-format off
    tft_init_config_t config = {
        .rotation = TFT_Landscape, 
        .display_config = &gc9a01_display_config
    };
    // clang-format on

    active_gfx = tft_init(config);

    if (active_gfx->error != PLATFORM_OK) {
        platform_log(PLATFORM_LOG_LEVEL_ERROR, TAG, "Failed to initialize TFT: %d", active_gfx->error);
        return;
    }

    cube_scene.scene_init(&gc9a01_display_config);

    // FPS text should be added last
    scene_object_t *fps_text = active_gfx->gfx_add_text(40, 35, "FPS: 0", 1, get_color565(255, 255, 255), TEXT_ALIGN_LEFT, TEXT_ALIGN_TOP);

    active_gfx->gfx_render();

    while (1) {
        cube_scene.scene_update(NULL);

        app_do_fps(fps_text);
        active_gfx->gfx_render();
    }
}
