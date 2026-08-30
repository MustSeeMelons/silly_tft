# Silly TFT

Started as a basic ILI9341 ESP driver/graphics playground and morphed into a agnostic display module library.

Options are exposed once the need arises.

Why silly? There are other libraries that do the same thing with more features and optimization - this is a playground.

## Display modules

Currently works with these modules. Other modules can be added by implementing the `display_config_t` struct.

### ILI9341

### GC9A01

## Graphics

Implements a scanline rasterizer which can:

- Render rectangles
- Circles
- Bitmap fonts
- Lines
- Triangles

Upon adding something you get a handle to edit the objects properties. 
Animations can be done using scenes, see `scene_t`.

Other rasterizers can be added by implementing the `gfx_t` struct.

## Datasheet

ESP32: https://documentation.espressif.com/esp32_technical_reference_manual_en.pdf

## Structure

- `core/include/spi_display_t.h`: transport layer defintion, like SPI
- `core/include/display.h`: display driver module flow implementation
- `core/include/gfx.h`: graphics context definition, like the scanline one
- `core/include/platform.h`: MCU specific necessary callbacks, like delay, timestamps, logging

# Logic Analyzer

Channel 0 = Clock

Channel 1 = MISO

Channel 2 = DCX

Channel 3 = CX

# Future?

- Positions for each scene object (some already have this) for easy movement across the screen
- Polygons (would be required if we wish to rotate a rectangle)
- Object rotation (Triangles/Polygons)