#ifndef SPI_DISPLAY_H_
#define SPI_DISPLAY_H_

#include "stdint.h"

// Defines a display driver protocol, technically may not be SPI at all
typedef struct {
    uint8_t (*spi_init)(void);
    void (*spi_send_command)(uint8_t cmd);
    void (*spi_send_data)(uint8_t *data, uint32_t len);
    void (*spi_send_data_byte)(uint8_t data);
} spi_display_t;

#endif