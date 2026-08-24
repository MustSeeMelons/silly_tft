#include "include/display.h"
#include "include/spi_display.h"
#include "include/tft.h"

#define C_SOFT_RESET                          0x01
#define C_READ_DISPLAY_ID                     0x04
#define C_READ_DISPLAY_STATUS                 0x09
#define C_READ_DISPLAY_POWER_MODE             0x0A
#define C_READ_DISPLAY_MADCTL                 0x0B
#define C_READ_DISPLAY_PIXEL_FORMAT           0x0C
#define C_READ_DISPLAY_IMAGE_MODE             0x0D
#define C_READ_DISPLAY_SIGNAL_MODE            0x0E
#define C_READ_DISPLAY_SELF_DIAGNOSTIC_RESULT 0x0F
#define C_SLEEP_ENTER                         0x10
#define C_SLEEP_OUT                           0x11
#define C_PARTIAL_MODE_ON                     0x12
#define C_NORMAL_DISPLAY_MODE_ON              0x13
#define C_INVERSION_OFF                       0x20
#define C_INVERSION_ON                        0x21
#define C_GAMMA_SET                           0x26
#define C_DISPLAY_OFF                         0x28
#define C_DISPLAY_ON                          0x29
#define C_COLUMN_ADDRESS_SET                  0x2A
#define C_PAGE_ADDRESS_SET                    0x2B
#define C_MEMORY_WRITE                        0x2C
#define C_COLOR_SET                           0x2D
#define C_MEMORY_READ                         0x2E
#define C_PARTIAL_AREA                        0x30
#define C_VERT_SCROLL_DEFINITION              0x33
#define C_TEARING_EFFECT_LINE_OFF             0x34
#define C_TEARING_EFFECT_LINE_ON              0x35
#define C_MEMORY_ACCESS_CONTROL               0x36
#define C_VERT_SCROLL_START_ADDRESS           0x37
#define C_IDLE_MODE_OFF                       0x38
#define C_IDLE_MODE_ON                        0x39
#define C_PIXEL_FORMAT_SET                    0x3A
#define C_WRITE_MEMORY_CONTINUE               0x3C
#define C_READ_MEMORY_CONTINUE                0x3E
#define C_SET_TEAR_SCANLINE                   0x44
#define C_GET_SCANLINE                        0x45
#define C_WRITE_DISPLAY_BRIGHTNESS            0x51
#define C_READ_DISPLAY_BRIGHTNESS             0x52
#define C_WRITE_CTRL_DISPLAY                  0x53
#define C_READ_CTRL_DISPLAY                   0x54
#define C_WRITE_CONTENT_ADAPT_BRIGHTNESS      0x55
#define C_READ_CONTENT_ADAPT_BRIGHTNESS       0x56
#define C_WRITE_CABC                          0x5E
#define C_READ_CABC                           0x5F
#define C_READ_ID1                            0xDA
#define C_READ_ID2                            0xDB
#define C_READ_ID3                            0xDC

#define C_RGB_INTERFACE_CONTROL      0xB0
#define C_FRAME_RATE_CONTROL_NORMAL  0xB1
#define C_FRAME_RATE_CONTROL_IDLE    0xB2
#define C_FRAME_RATE_CONTROL_PARTIAL 0xB3
#define C_DISPLAY_INVERSION_CONTROL  0xB4
#define C_BLANKING_PORCH_CONTROL     0xB5
#define C_DISPLAY_FUNCTION_CONTROL   0xB6
#define C_ENTRY_MODE_SET             0xB7
#define C_BACKLIGHT_CONTROL_1        0xB8
#define C_BACKLIGHT_CONTROL_2        0xB9
#define C_BACKLIGHT_CONTROL_3        0xBA
#define C_BACKLIGHT_CONTROL_4        0xBB
#define C_BACKLIGHT_CONTROL_5        0xBC
#define C_BACKLIGHT_CONTROL_6        0xBD
#define C_BACKLIGHT_CONTROL_7        0xBE
#define C_BACKLIGHT_CONTROL_8        0xBF
#define C_POWER_CONTROL_1            0xC0
#define C_POWER_CONTROL_2            0xC1
#define C_VCOM_CONTROL_1             0xC5
#define C_VCOM_CONTROL_2             0xC7
#define C_NV_MEMORY_WRITE            0xD0
#define C_NV_MEMORY_PROTECTION_KEY   0xD1
#define C_NV_MEMORY_STATUS_READ      0xD2
#define C_READ_ID4                   0xD3
#define C_POSITIVE_GAMMA_CORRECTION  0xE0
#define C_NEGATIVE_GAMMA_CORRECTION  0xE1
#define C_DIGITAL_GAMMA_CONTROL_1    0xE2
#define C_DIGITAL_GAMMA_CONTROL_2    0xE3
#define C_INTERFACE_CONTROL          0xF6
#define C_POWER_CONTROL_A            0xCB
#define C_POWER_CONTROL_B            0xCF
#define C_DRIVER_TIMING_CONTROL_A    0xE8
#define C_DRIVER_TIMING_CONTROL_A_2  0xE9
#define C_DRIVER_TIMING_CONTROL_B    0xEA
#define C_POWER_ON_SEQUENCE_CONTROL  0xED
#define C_ENABLE_3G                  0xF2
#define C_PUMP_RATIO_CONTROL         0xF7

#define TFT_HEIGHT 320
#define TFT_WIDTH  240

extern spi_display_t *spi_display;

uint16_t curr_display_height;
uint16_t curr_display_width;

lcd_init_cmd_t ili_init_cmds[] = {
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

static void ili9341_set_command_data(uint8_t command_id, uint8_t *data, uint8_t data_len) {
    for (size_t i = 0; i < sizeof(ili_init_cmds) / sizeof(ili_init_cmds[0]); i++) {
        if (ili_init_cmds[i].cmd == command_id) {
            ili_init_cmds[i].dataCount = data_len;
            memcpy(ili_init_cmds[i].data, data, data_len);
            return;
        }
    }
}

void ili9341_patch_init_commands(tft_rotation_t rotation) {
    // clang-format off
    switch (rotation) {
    case TFT_Portrait:
        curr_display_width = TFT_WIDTH;
        curr_display_height = TFT_HEIGHT;

        ili9341_set_command_data(C_MEMORY_ACCESS_CONTROL, (uint8_t[]){TFT_Portrait}, 1);
        break;
    case TFT_Landscape:
        curr_display_width = TFT_HEIGHT;
        curr_display_height = TFT_WIDTH;

        ili9341_set_command_data(C_MEMORY_ACCESS_CONTROL, (uint8_t[]){TFT_Landscape}, 1);
        break;
    case TFT_Portrait_Flipped:
        curr_display_width = TFT_WIDTH;
        curr_display_height = TFT_HEIGHT;

        ili9341_set_command_data(C_MEMORY_ACCESS_CONTROL, (uint8_t[]){TFT_Portrait_Flipped}, 1); 
        break;
    case TFT_Landscape_Flipped:
        curr_display_width = TFT_HEIGHT;
        curr_display_height = TFT_WIDTH;

        ili9341_set_command_data(C_MEMORY_ACCESS_CONTROL, (uint8_t[]){TFT_Landscape_Flipped}, 1);
        break;
    default:
        break;
    }
    // clang-format on
}

void ili9341_set_window(int16_t x_start, int16_t y_start, int16_t x_end, int16_t y_end) {
    spi_display->spi_send_command(C_COLUMN_ADDRESS_SET);
    spi_display->spi_send_data_byte(x_start >> 8);
    spi_display->spi_send_data_byte(x_start & 0xFF);
    spi_display->spi_send_data_byte(x_end >> 8);
    spi_display->spi_send_data_byte(x_end & 0xFF);

    spi_display->spi_send_command(C_PAGE_ADDRESS_SET);
    spi_display->spi_send_data_byte(y_start >> 8);
    spi_display->spi_send_data_byte(y_start & 0xFF);
    spi_display->spi_send_data_byte(y_end >> 8);
    spi_display->spi_send_data_byte(y_end & 0xFF);

    spi_display->spi_send_command(C_MEMORY_WRITE);
}

lcd_init_cmd_t *ili9341_init_commands(tft_rotation_t rotation) {
    ili9341_patch_init_commands(rotation);
    return ili_init_cmds;
}

uint16_t ili9341_get_width() {
    return curr_display_width;
}

uint16_t ili9341_get_height() {
    return curr_display_height;
}

display_config_t ili9341_display_config = {
    .display_init_commands = ili9341_init_commands,
    .set_window = ili9341_set_window,
    .get_width = ili9341_get_width,
    .get_height = ili9341_get_height,
};