idf_component_register(
    SRCS 
    "core/utils.c"
    "core/math_functions.c"
    "core/font.c"
    "core/tft.c"
    "main.c"
    "core/gfx/scanline_gfx.c"
    "core/gfx/gfx.c"
    # "core/scenes/egg_fall_scene.c"
    # "core/scenes/line_scene.c"
    # "core/scenes/triangles_scene.c"
    "core/scenes/cube.c"
    # "core/display/ili9341.c"
    "core/display/gc9a01.c"
    "spi_display.c"
    INCLUDE_DIRS "."
)
