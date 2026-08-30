#include "core/include/display.h"
#include "core/include/spi_display.h"
#include "core/include/tft.h"

#define GC9A01A_SWRESET   0x01 ///< Software Reset (maybe, not documented)
#define GC9A01A_RDDID     0x04 ///< Read display identification information
#define GC9A01A_RDDST     0x09 ///< Read Display Status
#define GC9A01A_SLPIN     0x10 ///< Enter Sleep Mode
#define GC9A01A_SLPOUT    0x11 ///< Sleep Out
#define GC9A01A_PTLON     0x12 ///< Partial Mode ON
#define GC9A01A_NORON     0x13 ///< Normal Display Mode ON
#define GC9A01A_INVOFF    0x20 ///< Display Inversion OFF
#define GC9A01A_INVON     0x21 ///< Display Inversion ON
#define GC9A01A_DISPOFF   0x28 ///< Display OFF
#define GC9A01A_DISPON    0x29 ///< Display ON
#define GC9A01A_CASET     0x2A ///< Column Address Set
#define GC9A01A_RASET     0x2B ///< Row Address Set
#define GC9A01A_RAMWR     0x2C ///< Memory Write
#define GC9A01A_PTLAR     0x30 ///< Partial Area
#define GC9A01A_VSCRDEF   0x33 ///< Vertical Scrolling Definition
#define GC9A01A_TEOFF     0x34 ///< Tearing Effect Line OFF
#define GC9A01A_TEON      0x35 ///< Tearing Effect Line ON
#define GC9A01A_MADCTL    0x36 ///< Memory Access Control
#define GC9A01A_VSCRSADD  0x37 ///< Vertical Scrolling Start Address
#define GC9A01A_IDLEOFF   0x38 ///< Idle mode OFF
#define GC9A01A_IDLEON    0x39 ///< Idle mode ON
#define GC9A01A_COLMOD    0x3A ///< Pixel Format Set
#define GC9A01A_CONTINUE  0x3C ///< Write Memory Continue
#define GC9A01A_TEARSET   0x44 ///< Set Tear Scanline
#define GC9A01A_GETLINE   0x45 ///< Get Scanline
#define GC9A01A_SETBRIGHT 0x51 ///< Write Display Brightness
#define GC9A01A_SETCTRL   0x53 ///< Write CTRL Display
#define GC9A01A1_POWER7   0xA7 ///< Power Control 7
#define GC9A01A_TEWC      0xBA ///< Tearing effect width control
#define GC9A01A1_POWER1   0xC1 ///< Power Control 1
#define GC9A01A1_POWER2   0xC3 ///< Power Control 2
#define GC9A01A1_POWER3   0xC4 ///< Power Control 3
#define GC9A01A1_POWER4   0xC9 ///< Power Control 4
#define GC9A01A_RDID1     0xDA ///< Read ID 1
#define GC9A01A_RDID2     0xDB ///< Read ID 2
#define GC9A01A_RDID3     0xDC ///< Read ID 3
#define GC9A01A_FRAMERATE 0xE8 ///< Frame rate control
#define GC9A01A_SPI2DATA  0xE9 ///< SPI 2DATA control
#define GC9A01A_INREGEN2  0xEF ///< Inter register enable 2
#define GC9A01A_GAMMA1    0xF0 ///< Set gamma 1
#define GC9A01A_GAMMA2    0xF1 ///< Set gamma 2
#define GC9A01A_GAMMA3    0xF2 ///< Set gamma 3
#define GC9A01A_GAMMA4    0xF3 ///< Set gamma 4
#define GC9A01A_IFACE     0xF6 ///< Interface control
#define GC9A01A_INREGEN1  0xFE ///< Inter register enable 1

#define TFT_HEIGHT 240
#define TFT_WIDTH  240

// MADCTL values for GC9A01 with RGB color order (BGR bit 3 = 0)
#define GC9A01_MADCTL_PORTRAIT          0x40
#define GC9A01_MADCTL_LANDSCAPE         0x20
#define GC9A01_MADCTL_PORTRAIT_FLIPPED  0x80
#define GC9A01_MADCTL_LANDSCAPE_FLIPPED 0xE0

extern spi_display_t *spi_display;

static uint16_t curr_display_height = 240;
static uint16_t curr_display_width = 240;

// clang-format off
lcd_init_cmd_t gc9a01_init_cmds[] = {
    {GC9A01A_INREGEN2, {0}, 0},
    {0xEB, {0x14}, 1}, // ?
    {GC9A01A_INREGEN1, {0}, 0},
    {GC9A01A_INREGEN2, {0}, 0},
    {0xEB, {0x14}, 1}, // ?
    {0x84, {0x40}, 1}, // ?
    {0x85, {0xFF}, 1}, // ?
    {0x86, {0xFF}, 1}, // ?
    {0x87, {0xFF}, 1}, // ?
    {0x88, {0x0A}, 1}, // ?
    {0x89, {0x21}, 1}, // ?
    {0x8A, {0x00}, 1}, // ?
    {0x8B, {0x80}, 1}, // ?
    {0x8C, {0x01}, 1}, // ?
    {0x8D, {0x01}, 1}, // ?
    {0x8E, {0xFF}, 1}, // ?
    {0x8F, {0xFF}, 1}, // ?
    {0xB6, {0x00, 0x00}, 2}, // ?
    {GC9A01A_MADCTL, {GC9A01_MADCTL_PORTRAIT}, 1},
    {GC9A01A_COLMOD, {0x05}, 1},
    {0x90, {0x08, 0x08, 0x08, 0x08}, 4}, // ?
    {0xBD, {0x06}, 1}, // ?
    {0xBC, {0x00}, 1}, // ?
    {0xFF, {0x60, 0x01, 0x04}, 3}, // ?
    {GC9A01A1_POWER2, {0x13}, 1},
    {GC9A01A1_POWER3, {0x13}, 1},
    {GC9A01A1_POWER4, {0x22}, 1},
    {0xBE, {0x11}, 1}, // ?
    {0xE1, {0x10, 0x0E}, 2}, // ?
    {0xDF, {0x21, 0x0c, 0x02}, 3}, // ?
    {GC9A01A_GAMMA1, {0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6},
    {GC9A01A_GAMMA2, {0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6},
    {GC9A01A_GAMMA3, {0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6},
    {GC9A01A_GAMMA4, {0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6},
    {0xED, {0x1B, 0x0B}, 2}, // ?
    {0xAE, {0x77}, 1}, // ?
    {0xCD, {0x63}, 1}, // ?
    // Unsure what this line (from manufacturer's boilerplate code) is
    // meant to do, but users reported issues, seems to work OK without:
    // {0x70, {0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0x08, 0x03}, 9}, // ?
    {GC9A01A_FRAMERATE, {0x34}, 1},
    {0x62, {0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70}, 12}, // ?
    {0x63, {0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xF3, 0x70, 0x70}, 12}, // ?
    {0x64, {0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07}, 7}, // ?
    {0x66, {0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00}, 10}, // ?
    {0x67, {0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98}, 10}, // ?
    {0x74, {0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00}, 7}, // ?
    {0x98, {0x3e, 0x07}, 2}, // ?
    {GC9A01A_TEON, {0}, 0},
    {GC9A01A_INVON, {0}, 0},
    {GC9A01A_SLPOUT, {0}, 0x80}, // Exit sleep
    {GC9A01A_DISPON, {0}, 0x80}, // Display on
    {0, {0}, 0xff},              // End marker
};
// clang-format on

static void gc9a01_set_command_data(uint8_t command_id, uint8_t *data, uint8_t data_len) {
    for (size_t i = 0; i < sizeof(gc9a01_init_cmds) / sizeof(gc9a01_init_cmds[0]); i++) {
        if (gc9a01_init_cmds[i].cmd == command_id) {
            gc9a01_init_cmds[i].dataCount = data_len;
            memcpy(gc9a01_init_cmds[i].data, data, data_len);
            return;
        }
    }
}

void gc9a01_patch_init_commands(tft_rotation_t rotation) {
    // clang-format off
    switch (rotation) {
    case TFT_Portrait:
        gc9a01_set_command_data(GC9A01A_MADCTL, (uint8_t[]){GC9A01_MADCTL_PORTRAIT}, 1);
        break;
    case TFT_Landscape:
        gc9a01_set_command_data(GC9A01A_MADCTL, (uint8_t[]){GC9A01_MADCTL_LANDSCAPE}, 1);
        break;
    case TFT_Portrait_Flipped:
        gc9a01_set_command_data(GC9A01A_MADCTL, (uint8_t[]){GC9A01_MADCTL_PORTRAIT_FLIPPED}, 1);
        break;
    case TFT_Landscape_Flipped:
        gc9a01_set_command_data(GC9A01A_MADCTL, (uint8_t[]){GC9A01_MADCTL_LANDSCAPE_FLIPPED}, 1);
        break;
    default:
        break;
    }
    // clang-format on
}

lcd_init_cmd_t *gc9a01_init_commands(tft_rotation_t rotation) {
    gc9a01_patch_init_commands(rotation);
    return gc9a01_init_cmds;
}

void gc9a01_set_window(int16_t x_start, int16_t y_start, int16_t x_end, int16_t y_end) {
    spi_display->spi_send_command(GC9A01A_CASET);
    spi_display->spi_send_data_byte(x_start >> 8);
    spi_display->spi_send_data_byte(x_start & 0xFF);
    spi_display->spi_send_data_byte(x_end >> 8);
    spi_display->spi_send_data_byte(x_end & 0xFF);

    spi_display->spi_send_command(GC9A01A_RASET);
    spi_display->spi_send_data_byte(y_start >> 8);
    spi_display->spi_send_data_byte(y_start & 0xFF);
    spi_display->spi_send_data_byte(y_end >> 8);
    spi_display->spi_send_data_byte(y_end & 0xFF);

    spi_display->spi_send_command(GC9A01A_RAMWR);
}

uint16_t gc9a01_get_width() {
    return curr_display_width;
}

uint16_t gc9a01_get_height() {
    return curr_display_height;
}

display_config_t gc9a01_display_config = {
    .display_init_commands = gc9a01_init_commands,
    .set_window = gc9a01_set_window,
    .get_width = gc9a01_get_width,
    .get_height = gc9a01_get_height,
};