
#include "esp_log.h"
#include "esp_timer.h"
#include "include/defs.h"
#include "include/scene.h"
#include "include/spi_display.h"
#include "include/tft.h"
#include "include/utils.h"
#include <stdio.h>

static const char *TAG = "Main";

static void app_do_fps(scene_object_t *text) {
    static uint32_t last_frame = 0;
    static float fps = 0.0f;

    int64_t now = esp_timer_get_time() / 1000;

    if (last_frame != 0) {
        uint32_t frame_time = now - last_frame;

        if (frame_time > 0) {
            float current_fps = 1000.0f / frame_time;

            // Smooth out sudden changes
            fps = fps * 0.7f + current_fps * 0.3f;
        }

        static char fps_text[32];
        snprintf(fps_text, sizeof(fps_text), "%.1f FPS (%lu ms)", fps, frame_time);
        text->text.text = fps_text;
    }

    last_frame = now;
}

extern scene_t egg_fall_scene;
extern scene_t triangles_scene;
extern scene_t line_scene;
extern scene_t cube_scene;

// The config to use
extern display_config_t ili9341_display_config;

extern spi_display_t esp_spi;
spi_display_t *spi_display = &esp_spi;

gfx_t *active_gfx;

void app_main(void) {
    esp_err_t err = esp_spi.spi_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI: %s", esp_err_to_name(err));
        return;
    }

    // clang-format off
    tft_init_config_t config = {
        .rotation = TFT_Landscape, 
        .display_config = &ili9341_display_config
    };
    // clang-format on

    active_gfx = tft_init(config);

    if (active_gfx->error != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize TFT: %s", esp_err_to_name(active_gfx->error));
        return;
    }

    cube_scene.scene_init(NULL);

    // FPS text should be added last
    scene_object_t *fps_text = active_gfx->gfx_add_text(0, 0, "FPS: 0", 1, get_color565(255, 255, 255));

    active_gfx->gfx_render();

    while (1) {
        cube_scene.scene_update(NULL);

        app_do_fps(fps_text);
        active_gfx->gfx_render();
    }
}
