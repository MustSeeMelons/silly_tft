#include "core/include/spi_display.h"
#include "core/include/defs.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

// GPIO Matrix
#define GPIO_BASE             0x3FF44000
#define GPIO_OUT_REG          ((volatile uint32_t *)(GPIO_BASE + 0x0004)) // Current state ...31
#define GPIO_OUT_W1TS_REG     ((volatile uint32_t *)(GPIO_BASE + 0x0008)) // Write 1 to set output
#define GPIO_OUT_W1TC_REG     ((volatile uint32_t *)(GPIO_BASE + 0x000C)) // Write 1 to clear output
#define GPIO_OUT1_REG         ((volatile uint32_t *)(GPIO_BASE + 0x0010)) // 32...
#define GPIO_OUT1_W1TS_REG    ((volatile uint32_t *)(GPIO_BASE + 0x0014))
#define GPIO_OUT1_W1TC_REG    ((volatile uint32_t *)(GPIO_BASE + 0x0018))
#define GPIO_ENABLE_REG       ((volatile uint32_t *)(GPIO_BASE + 0x0020)) // Current state
#define GPIO_ENABLE_W1TS_REG  ((volatile uint32_t *)(GPIO_BASE + 0x0024)) // Write 1 to set output
#define GPIO_ENABLE_W1TC_REG  ((volatile uint32_t *)(GPIO_BASE + 0x0028)) // Write 1 to clear output
#define GPIO_ENABLE1_REG      ((volatile uint32_t *)(GPIO_BASE + 0x002C)) // 32...
#define GPIO_ENABLE1_W1TS_REG ((volatile uint32_t *)(GPIO_BASE + 0x0030))
#define GPIO_ENABLE1_W1TC_REG ((volatile uint32_t *)(GPIO_BASE + 0x0034))

// IO Mux
#define IO_MUX_BASE 0x3FF49000

#define TFT_CS    4 // CSX
#define TFT_RESET 5
#define TFT_DC    15 // D/CX Data/command. High = data, Low = command
#define TFT_MOSI  13
#define TFT_SCK   14 // SCL Clock
#define TFT_LED   23 // Backlight
#define TFT_MISO  12

static spi_device_handle_t spi_handle;

static platform_err_t spi_init_pins(void) {
    platform_err_t err = PLATFORM_OK;

    gpio_config_t io_led = {
        .pin_bit_mask = 1ULL << TFT_LED,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    err = gpio_config(&io_led);

    if (err != PLATFORM_OK) {
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

    if (err != PLATFORM_OK) {
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

    if (err != PLATFORM_OK) {
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

static platform_err_t spi_init(void) {
    uint8_t err = 0;

    spi_bus_config_t buscfg = {
        .mosi_io_num = TFT_MOSI,
        .miso_io_num = TFT_MISO,
        .sclk_io_num = TFT_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 320 * 32 * 2,
    };

    err = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    if (err != PLATFORM_OK) {
        return err;
    }

    spi_device_interface_config_t devcfg = {
        // Up to 34 Mhz is fine
        .clock_speed_hz = 34 * 1000 * 1000, // 34 MHz
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1,
        .flags = 0,
    };

    err = spi_init_pins();

    // Hardware reset the driver as per the datasheet
    gpio_set_level(TFT_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(TFT_RESET, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(TFT_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    err = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle);

    return err;
}

static void spi_send_data_byte(uint8_t data) {
    *GPIO_OUT_W1TC_REG = (0x1 << TFT_CS);
    *GPIO_OUT_W1TS_REG = (0x1 << TFT_DC);

    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data,
    };

    spi_device_transmit(spi_handle, &t);

    *GPIO_OUT_W1TS_REG = (0x1 << TFT_CS);
}

static void spi_send_command(uint8_t cmd) {
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

static void spi_send_data(uint8_t *data, uint32_t len) {
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

spi_display_t esp_spi = {
    .spi_init = spi_init,
    .spi_send_command = spi_send_command,
    .spi_send_data = spi_send_data,
    .spi_send_data_byte = spi_send_data_byte,
};