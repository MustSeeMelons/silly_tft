#include "include/tft.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_random.h"
#include "include/defs.h"
#include "include/gfx.h"
#include "include/utils.h"
#include "math.h"
#include <string.h>

uint16_t curr_display_height;
uint16_t curr_display_width;

extern gfx_t scanline_gfx;

static lcd_init_cmd_t ili_init_cmds[] = {
    {C_POWER_CONTROL_B, {0x00, 0x83, 0X30}, 3},
    {C_POWER_ON_SEQUENCE_CONTROL, {0x64, 0x03, 0X12, 0X81}, 4},
    {C_DRIVER_TIMING_CONTROL_A, {0x85, 0x01, 0x79}, 3},
    {C_POWER_CONTROL_A, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    {C_PUMP_RATIO_CONTROL, {0x30}, 1},
    {C_DRIVER_TIMING_CONTROL_B, {0x00, 0x00}, 2},
    {C_POWER_CONTROL_1, {0x26}, 1},
    {C_POWER_CONTROL_2, {0x11}, 1},
    {C_VCOM_CONTROL_1, {0x35, 0x3E}, 2},
    {C_VCOM_CONTROL_2, {0xBE}, 1},
    {C_MEMORY_ACCESS_CONTROL, {TFT_Portrait}, 1},
    {C_PIXEL_FORMAT_SET, {0x55}, 1}, // 16 bit for RGB and MCU interfac
    {C_FRAME_RATE_CONTROL_NORMAL, {0x00, 0x1B}, 2},
    {C_ENABLE_3G, {0x02}, 1}, // Disable extra gamma correction
    {C_GAMMA_SET, {0x02}, 1},
    // Gamma corrections are some defaults from samples
    {C_POSITIVE_GAMMA_CORRECTION,
     {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00},
     15},
    {C_NEGATIVE_GAMMA_CORRECTION,
     {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F},
     15},
    {C_ENTRY_MODE_SET, {0x07}, 1},
    {C_DISPLAY_FUNCTION_CONTROL, {0x0A, 0x82, 0x27, 0x00}, 4},
    // Default sequnce as per datasheet
    {C_SLEEP_OUT, {0}, 0x80},
    {C_DISPLAY_ON, {0}, 0x80},
    {C_COLUMN_ADDRESS_SET, {0, 0, (TFT_WIDTH - 1) >> 8, (TFT_WIDTH - 1) & 0xFF}, 4},
    {C_PAGE_ADDRESS_SET, {0, 0, (TFT_HEIGHT - 1) >> 8, (TFT_HEIGHT - 1) & 0xFF}, 4},
    {C_MEMORY_WRITE, {0}, 0},
    {0, {0}, 0xff}, // End marker
};

static spi_device_handle_t spi_handle;

static void tft_set_command_data(uint8_t command_id, uint8_t *data, uint8_t data_len) {
    for (size_t i = 0; i < sizeof(ili_init_cmds) / sizeof(ili_init_cmds[0]); i++) {
        if (ili_init_cmds[i].cmd == command_id) {
            ili_init_cmds[i].dataCount = data_len;
            memcpy(ili_init_cmds[i].data, data, data_len);
            return;
        }
    }
}

void tft_patch_init_commands(tft_rotation_t rotation) {
    // clang-format off
    switch (rotation) {
    case TFT_Portrait:
        curr_display_width = TFT_WIDTH;
        curr_display_height = TFT_HEIGHT;

        tft_set_command_data(C_MEMORY_ACCESS_CONTROL, (uint8_t[]){TFT_Portrait}, 1);
        break;
    case TFT_Landscape:
        curr_display_width = TFT_HEIGHT;
        curr_display_height = TFT_WIDTH;

        tft_set_command_data(C_MEMORY_ACCESS_CONTROL, (uint8_t[]){TFT_Landscape}, 1);
        break;
    case TFT_Portrait_Flipped:
        curr_display_width = TFT_WIDTH;
        curr_display_height = TFT_HEIGHT;

        tft_set_command_data(C_MEMORY_ACCESS_CONTROL, (uint8_t[]){TFT_Portrait_Flipped}, 1); 
        break;
    case TFT_Landscape_Flipped:
        curr_display_width = TFT_HEIGHT;
        curr_display_height = TFT_WIDTH;

        tft_set_command_data(C_MEMORY_ACCESS_CONTROL, (uint8_t[]){TFT_Landscape_Flipped}, 1);
        break;
    default:
        break;
    }
    // clang-format on
}

/** Initialize non SPI TFT pins */
static esp_err_t tft_init_pins(void) {
    esp_err_t err;

    gpio_config_t io_led = {
        .pin_bit_mask = 1ULL << TFT_LED,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    err = gpio_config(&io_led);

    if (err != ESP_OK) {
        return err;
    }

    gpio_config_t io_cs = {
        .pin_bit_mask = 1ULL << TFT_CS,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    err = gpio_config(&io_cs);

    if (err != ESP_OK) {
        return err;
    }

    gpio_config_t io_dc = {
        .pin_bit_mask = 1ULL << TFT_DC,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    err = gpio_config(&io_dc);

    if (err != ESP_OK) {
        return err;
    }

    gpio_config_t io_reset = {
        .pin_bit_mask = 1ULL << TFT_RESET,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    err = gpio_config(&io_reset);

    gpio_set_level(TFT_CS, 1);
    gpio_set_level(TFT_DC, 1);

    return err;
}

void tft_send_data_byte(uint8_t data) {
    *GPIO_OUT_W1TC_REG = (0x1 << TFT_CS);
    *GPIO_OUT_W1TS_REG = (0x1 << TFT_DC);

    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data,
    };

    spi_device_transmit(spi_handle, &t);

    *GPIO_OUT_W1TS_REG = (0x1 << TFT_CS);
}

void tft_send_data(uint8_t *data, size_t len) {
    if (len == 0) {
        return;
    }

    // XXX Raw registers made no FPS difference
    *GPIO_OUT_W1TC_REG = (0x1 << TFT_CS);
    *GPIO_OUT_W1TS_REG = (0x1 << TFT_DC);

    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
    };

    spi_device_transmit(spi_handle, &t);

    *GPIO_OUT_W1TS_REG = (0x1 << TFT_CS);
}

void tft_send_command(uint8_t cmd) {
    *GPIO_OUT_W1TC_REG = (0x1 << TFT_CS);
    *GPIO_OUT_W1TC_REG = (0x1 << TFT_DC);

    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
    };

    spi_device_transmit(spi_handle, &t);

    *GPIO_OUT_W1TS_REG = (0x1 << TFT_CS);
    *GPIO_OUT_W1TS_REG = (0x1 << TFT_DC);
}

void tft_set_window(int16_t x_start, int16_t y_start, int16_t x_end, int16_t y_end) {
    tft_send_command(C_COLUMN_ADDRESS_SET);
    tft_send_data_byte(x_start >> 8);
    tft_send_data_byte(x_start & 0xFF);
    tft_send_data_byte(x_end >> 8);
    tft_send_data_byte(x_end & 0xFF);

    tft_send_command(C_PAGE_ADDRESS_SET);
    tft_send_data_byte(y_start >> 8);
    tft_send_data_byte(y_start & 0xFF);
    tft_send_data_byte(y_end >> 8);
    tft_send_data_byte(y_end & 0xFF);

    tft_send_command(C_MEMORY_WRITE);
}

gfx_t *tft_init(tft_init_config_t config) {
    esp_err_t err;

    // Apply rotation to init commands
    tft_patch_init_commands(config.rotation);

    spi_bus_config_t buscfg = {
        .mosi_io_num = TFT_MOSI,
        .miso_io_num = TFT_MISO,
        .sclk_io_num = TFT_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        // NOTE: This should change depending on gfx context used
        .max_transfer_sz = TFT_HEIGHT * 32 * 2,
    };

    err = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    if (err != ESP_OK) {
        scanline_gfx.error = err;
        return &scanline_gfx;
    }

    spi_device_interface_config_t devcfg = {
        // Up to 34 Mhz is fine
        .clock_speed_hz = 34 * 1000 * 1000, // 34 MHz
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1,
        .flags = 0,
    };

    err = tft_init_pins();

    if (err != ESP_OK) {
        scanline_gfx.error = err;
        return &scanline_gfx;
    }

    // Hardware reset the driver as per the datasheet
    gpio_set_level(TFT_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(TFT_RESET, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(TFT_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    err = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle);

    if (err != ESP_OK) {
        scanline_gfx.error = err;
        return &scanline_gfx;
    }

    gpio_set_level(TFT_LED, 0);

    uint8_t init_cmd_index = 0;
    lcd_init_cmd_t *lcd_init_cmds = ili_init_cmds;

    // Power ON sequence
    while (lcd_init_cmds[init_cmd_index].dataCount != 0xff) {
        tft_send_command(lcd_init_cmds[init_cmd_index].cmd);

        uint8_t data_len = lcd_init_cmds[init_cmd_index].dataCount & 0x1F; // 5 bits have length
        if (data_len > 0) {
            tft_send_data(lcd_init_cmds[init_cmd_index].data, data_len);
        }

        if (lcd_init_cmds[init_cmd_index].dataCount & 0x80) { // High bit signals delay
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        init_cmd_index++;
    }

    gpio_set_level(TFT_LED, 1);

    return &scanline_gfx;
}

void tft_reset() {
    scanline_gfx.gfx_reset();
    tft_clear();
}

void tft_clear() {
    tft_set_window(0, 0, curr_display_width - 1, curr_display_height - 1);

    uint16_t color = 0x0000;

    // Use the largest axis
    uint8_t line[curr_display_width * 2];

    // XXX Line by line is inneficent
    for (int x = 0; x < curr_display_width; x++) {
        line[x * 2] = color >> 8;
        line[x * 2 + 1] = color & 0xFF;
    }

    for (int y = 0; y < curr_display_height; y++) {
        tft_send_data(line, sizeof(line));
    }
}
