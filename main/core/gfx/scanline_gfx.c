#include "core/include/defs.h"
#include "core/include/display.h"
#include "core/include/gfx.h"
#include "core/include/math_functions.h"
#include "core/include/platform.h"
#include "core/include/spi_display.h"
#include "core/include/tft.h"
#include "core/include/utils.h"
#include <math.h>
#include <stdlib.h>

static const char *TAG = "SCANLINE_GFX";
#define TFT_SCANLINE_HEIGHT 32

extern const uint8_t font[96][32];
extern spi_display_t *spi_display;

extern uint16_t scene_object_id_counter;
extern uint8_t scene_index;

extern gfx_t scanline_gfx;

// Each pixel is a uint_16, 565 bit colors
// heap_caps_alloc made it slower for some reason
static uint8_t *scanline = NULL;

// Refs to all active objects
static scene_object_t scene_objects[GFX_SCENE_OBJECT_MAX];
// Ids of objects currently on each scanline, using larges axis
static scene_object_t *(*scanline_residents)[GFX_SCENE_OBJECT_MAX] = NULL;

static display_config_t *display_config = NULL;

static inline uint16_t get_scanline_count(void) {
    if (display_config == NULL) {
        return 0;
    }

    // Integer ceiling division
    return (display_config->get_height() + TFT_SCANLINE_HEIGHT - 1) / TFT_SCANLINE_HEIGHT;
}

static inline size_t get_scanline_residents_size(void) {
    return get_scanline_count() * sizeof(*scanline_residents);
}

void scanline_global_update() {
    if (scanline_residents == NULL) {
        return;
    }

    memset(scanline_residents, 0, get_scanline_residents_size());
    uint16_t total_scanlines = get_scanline_count();

    for (uint8_t i = 0; i < scene_index; i++) {
        scene_object_t *obj = &scene_objects[i];

        int16_t start_scanline = obj->y / TFT_SCANLINE_HEIGHT;
        int16_t end_scanline = 0;

        switch (obj->type) {
        case OBJECT_RECTANGLE:
            end_scanline = (obj->y + obj->rectangle.height) / TFT_SCANLINE_HEIGHT;
            break;
        case OBJECT_TEXT:
            end_scanline = (obj->y + FONT_HEIGHT * obj->text.font_size) / TFT_SCANLINE_HEIGHT;
            break;
        case OBJECT_CIRCLE:
            start_scanline = (obj->y - obj->circle.radius) / TFT_SCANLINE_HEIGHT;
            end_scanline = (obj->y + obj->circle.radius) / TFT_SCANLINE_HEIGHT;
            break;
        case OBJECT_LINE: {
            int16_t line_min_y = MIN(obj->y, obj->line.y_end);
            int16_t line_max_y = MAX(obj->y, obj->line.y_end);
            start_scanline = line_min_y / TFT_SCANLINE_HEIGHT;
            end_scanline = line_max_y / TFT_SCANLINE_HEIGHT;
            break;
        }
        case OBJECT_TRIANGLE:
            start_scanline = obj->triangle.min_y / TFT_SCANLINE_HEIGHT;
            end_scanline = obj->triangle.max_y / TFT_SCANLINE_HEIGHT;
            break;
        default:
            break;
        }

        for (int j = start_scanline; j <= end_scanline; j++) {
            if (j < 0 || j >= total_scanlines) {
                continue;
            }

            for (int k = 0; k < GFX_SCENE_OBJECT_MAX; k++) {
                if (scanline_residents[j][k] == 0) {
                    scanline_residents[j][k] = obj;
                    break;
                }
            }
        }
    }
}

void gfx_reset() {
    scene_index = 0;
    memset(scene_objects, 0, sizeof(scene_objects));

    if (scanline_residents != NULL) {
        memset(scanline_residents, 0, get_scanline_residents_size());
    }
}

scene_object_t *scanline_add_rectangle(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) {
    if (scene_index >= GFX_SCENE_OBJECT_MAX) {
        return NULL;
    }

    scene_object_t *obj = &scene_objects[scene_index++];

    obj->id = scene_object_id_counter++;
    obj->type = OBJECT_RECTANGLE;
    obj->x = x;
    obj->y = y;
    obj->rectangle.width = width;
    obj->rectangle.height = height;
    obj->rectangle.color = color;

    int16_t start_scanline = obj->y / TFT_SCANLINE_HEIGHT;
    int16_t end_scanline = (obj->y + obj->rectangle.height) / TFT_SCANLINE_HEIGHT;

    uint16_t total_scanlines = get_scanline_count();

    // Place it in the first empty slot
    for (int i = start_scanline; i <= end_scanline; i++) {
        if (i < 0 || i >= total_scanlines) {
            continue;
        }

        for (int j = 0; j < GFX_SCENE_OBJECT_MAX; j++) {
            if (scanline_residents[i][j] == 0) {
                scanline_residents[i][j] = obj;
                break;
            }
        }
    }

    return obj;
}

scene_object_t *scanline_add_circle(int16_t x, int16_t y, uint16_t radius, uint16_t color) {
    if (scene_index >= GFX_SCENE_OBJECT_MAX) {
        return NULL;
    }

    scene_object_t *obj = &scene_objects[scene_index++];

    obj->id = scene_object_id_counter++;
    obj->type = OBJECT_CIRCLE;
    obj->x = x;
    obj->y = y;
    obj->circle.radius = radius;
    obj->circle.color = color;

    int16_t start_scanline = (y - radius) / TFT_SCANLINE_HEIGHT;
    int16_t end_scanline = (y + radius) / TFT_SCANLINE_HEIGHT;

    uint16_t total_scanlines = get_scanline_count();

    // Place it in the first empty slot
    for (int16_t i = start_scanline; i <= end_scanline; i++) {
        if (i < 0 || i >= total_scanlines) {
            continue;
        }

        for (int j = 0; j < GFX_SCENE_OBJECT_MAX; j++) {
            if (scanline_residents[i][j] == 0) {
                scanline_residents[i][j] = obj;
                break;
            }
        }
    }

    return obj;
}

scene_object_t *scanline_add_text(int16_t x, int16_t y, char *text, uint16_t font_size, uint16_t color) {
    if (scene_index >= GFX_SCENE_OBJECT_MAX) {
        return NULL;
    }

    scene_object_t *obj = &scene_objects[scene_index++];

    obj->id = scene_object_id_counter++;
    obj->type = OBJECT_TEXT;
    obj->x = x;
    obj->y = y;
    obj->text.text = text;
    obj->text.font_size = font_size;
    obj->text.color = color;

    int16_t start_scanline = obj->y / TFT_SCANLINE_HEIGHT;
    int16_t end_scanline = (obj->y + FONT_HEIGHT * font_size) / TFT_SCANLINE_HEIGHT;

    uint16_t total_scanlines = get_scanline_count();

    for (int i = start_scanline; i <= end_scanline; i++) {
        if (i < 0 || i >= total_scanlines) {
            continue;
        }

        for (int j = 0; j < GFX_SCENE_OBJECT_MAX; j++) {
            if (scanline_residents[i][j] == 0) {
                scanline_residents[i][j] = obj;
                break;
            }
        }
    }

    return obj;
}

scene_object_t *scanline_add_line(
    int16_t x_start, int16_t y_start, int16_t x_end, int16_t y_end, uint16_t color, uint8_t stroke) {
    if (scene_index >= GFX_SCENE_OBJECT_MAX) {
        return NULL;
    }

    scene_object_t *obj = &scene_objects[scene_index++];

    obj->id = scene_object_id_counter++;
    obj->type = OBJECT_LINE;
    obj->x = x_start;
    obj->y = y_start;
    obj->line.x_end = x_end;
    obj->line.y_end = y_end;
    obj->line.color = color;
    obj->line.stroke = stroke;

    int16_t line_min_y = MIN(obj->y, obj->line.y_end);
    int16_t line_max_y = MAX(obj->y, obj->line.y_end);

    int16_t start_scanline = line_min_y / TFT_SCANLINE_HEIGHT;
    int16_t end_scanline = line_max_y / TFT_SCANLINE_HEIGHT;

    uint16_t total_scanlines = get_scanline_count();

    for (int i = start_scanline; i <= end_scanline; i++) {
        if (i < 0 || i >= total_scanlines) {
            continue;
        }

        for (int j = 0; j < GFX_SCENE_OBJECT_MAX; j++) {
            if (scanline_residents[i][j] == 0) {
                scanline_residents[i][j] = obj;
                break;
            }
        }
    }

    return obj;
}

static void scanline_triangle_update(scene_object_t *obj) {
    if (obj->type != OBJECT_TRIANGLE) {
        return;
    }

    obj->triangle.min_x = get_min_x(obj->triangle.vertices);
    obj->triangle.max_x = get_max_x(obj->triangle.vertices);
    obj->triangle.min_y = get_min_y(obj->triangle.vertices);
    obj->triangle.max_y = get_max_y(obj->triangle.vertices);

    // Edge functions are linear, so we can calcualte the step
    obj->triangle.ab.step_x = -(obj->triangle.vertices[1].y - obj->triangle.vertices[0].y);
    obj->triangle.ab.step_y = (obj->triangle.vertices[1].x - obj->triangle.vertices[0].x);

    obj->triangle.bc.step_x = -(obj->triangle.vertices[2].y - obj->triangle.vertices[1].y);
    obj->triangle.bc.step_y = (obj->triangle.vertices[2].x - obj->triangle.vertices[1].x);

    obj->triangle.ca.step_x = -(obj->triangle.vertices[0].y - obj->triangle.vertices[2].y);
    obj->triangle.ca.step_y = (obj->triangle.vertices[0].x - obj->triangle.vertices[2].x);
}

scene_object_t *scanline_add_triangle(vector2_t vertices[3], uint16_t color) {
    if (scene_index >= GFX_SCENE_OBJECT_MAX) {
        return NULL;
    }

    scene_object_t *obj = &scene_objects[scene_index++];
    obj->id = scene_object_id_counter++;
    obj->type = OBJECT_TRIANGLE;
    memcpy(obj->triangle.vertices, vertices, sizeof(obj->triangle.vertices));
    obj->triangle.color = color;

    scanline_triangle_update(obj);

    uint16_t total_scanlines = get_scanline_count();

    int16_t start_scanline = obj->triangle.min_y / TFT_SCANLINE_HEIGHT;
    int16_t end_scanline = obj->triangle.max_y / TFT_SCANLINE_HEIGHT;

    for (int i = start_scanline; i <= end_scanline; i++) {
        if (i < 0 || i >= total_scanlines) {
            continue;
        }

        for (int j = 0; j < GFX_SCENE_OBJECT_MAX; j++) {
            if (scanline_residents[i][j] == 0) {
                scanline_residents[i][j] = obj;
                break;
            }
        }
    }

    return obj;
}

static void scanline_line_low(
    int16_t scan_y_px_start, int16_t x_start, int16_t y_start, int16_t x_end, int16_t y_end, uint16_t color) {
    int16_t dx = x_end - x_start;
    int16_t dy = y_end - y_start;
    int16_t yi = 1;

    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }

    int16_t d = 2 * dy - dx;

    int16_t y = y_start;

    int16_t scanline_row = y_start - scan_y_px_start;

    uint16_t curr_display_width = display_config->get_width();

    if (x_start >= 0 && x_start < curr_display_width && scanline_row >= 0 && scanline_row < TFT_SCANLINE_HEIGHT) {
        scanline[(scanline_row * curr_display_width + x_start) * 2] = color >> 8;
        scanline[(scanline_row * curr_display_width + x_start) * 2 + 1] = color & 0xFF;
    }

    for (int16_t x = x_start; x < x_end; x++) {
        if (d > 0) {
            d = d + (2 * (dy - dx));
            y = y + yi;

        } else {
            d = d + 2 * dy;
        }

        scanline_row = y - scan_y_px_start;

        if (x >= 0 && x < curr_display_width && scanline_row >= 0 && scanline_row < TFT_SCANLINE_HEIGHT) {
            scanline[(scanline_row * curr_display_width + x) * 2] = color >> 8;
            scanline[(scanline_row * curr_display_width + x) * 2 + 1] = color & 0xFF;
        }
    }
}

static void scanline_line_high(
    int16_t scan_y_px_start, int16_t x_start, int16_t y_start, int16_t x_end, int16_t y_end, uint16_t color) {

    int16_t dx = x_end - x_start;
    int16_t dy = y_end - y_start;
    int16_t xi = 1;

    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }

    int16_t d = 2 * dx - dy;

    int16_t x = x_start;

    int16_t scanline_row = y_start - scan_y_px_start;

    uint16_t curr_display_width = display_config->get_width();

    if (x >= 0 && x < curr_display_width && scanline_row >= 0 && scanline_row < TFT_SCANLINE_HEIGHT) {
        scanline[(scanline_row * curr_display_width + x) * 2] = color >> 8;
        scanline[(scanline_row * curr_display_width + x) * 2 + 1] = color & 0xFF;
    }

    for (int16_t y = y_start; y < y_end; y++) {
        if (d > 0) {
            d = d + (2 * (dx - dy));
            x = x + xi;

        } else {
            d = d + 2 * dx;
        }

        scanline_row = y - scan_y_px_start;

        if (x >= 0 && x < curr_display_width && scanline_row >= 0 && scanline_row < TFT_SCANLINE_HEIGHT) {
            scanline[(scanline_row * curr_display_width + x) * 2] = color >> 8;
            scanline[(scanline_row * curr_display_width + x) * 2 + 1] = color & 0xFF;
        }
    }
}

static void scanline_render_line(
    int16_t scan_y_px_start, int16_t x_start, int16_t y_start, int16_t x_end, int16_t y_end, uint16_t color) {
    // Bresenham line algorithm straight from Wikipedia
    if (abs(y_end - y_start) < abs(x_end - x_start)) {
        if (x_start > x_end) {
            scanline_line_low(scan_y_px_start, x_end, y_end, x_start, y_start, color);
        } else {
            scanline_line_low(scan_y_px_start, x_start, y_start, x_end, y_end, color);
        }
    } else {
        if (y_start > y_end) {
            scanline_line_high(scan_y_px_start, x_end, y_end, x_start, y_start, color);
        } else {
            scanline_line_high(scan_y_px_start, x_start, y_start, x_end, y_end, color);
        }
    }
}

void scanline_update_scene_object(scene_object_t *obj) {
    switch (obj->type) {
    case OBJECT_TRIANGLE:
        scanline_triangle_update(obj);
        break;
    default:
        // Most dont need anything
        break;
    }
}

void scanline_render_scene() {
    uint16_t total_scanlines = get_scanline_count();
    uint16_t curr_display_width = display_config->get_width();
    uint16_t curr_display_height = display_config->get_height();

    // Go through each scanline
    for (size_t scanline_index = 0; scanline_index < total_scanlines; scanline_index++) {
        // Clear scanline
        int16_t scan_y_px_start = scanline_index * TFT_SCANLINE_HEIGHT;
        int16_t scanline_height = MIN(TFT_SCANLINE_HEIGHT, display_config->get_height() - scan_y_px_start);

        memset(scanline, 0x00, display_config->get_width() * scanline_height * 2);

        // Get the height in pixels so we can decide if we use object height or scanline height for rendering
        int16_t scan_y_px_end = scan_y_px_start + scanline_height;

        for (int j = 0; j < GFX_SCENE_OBJECT_MAX; j++) {
            // Render if we have a reference
            if (scanline_residents[scanline_index][j] != NULL) {
                scene_object_t *obj = scanline_residents[scanline_index][j];

                switch (obj->type) {
                case OBJECT_RECTANGLE: {
                    int16_t x_start = MAX(0, obj->x);
                    int16_t x_end = MIN(MAX(obj->x + obj->rectangle.width, 0), curr_display_width);

                    // clang-format off
                    int16_t y_start = obj->y > scan_y_px_start ? obj->y : scan_y_px_start;
                    int16_t y_end = MIN(obj->y + obj->rectangle.height < scan_y_px_end ? obj->y + obj->rectangle.height : scan_y_px_end, curr_display_height);
                    // clang-format on

                    // Fill object colors
                    for (int16_t y = y_start; y < y_end; y++) {
                        int16_t scanline_row = y - scan_y_px_start; // Which row of the scanline we are on

                        for (int16_t x = x_start; x < x_end; x++) {
                            scanline[(scanline_row * curr_display_width + x) * 2] = obj->rectangle.color >> 8;
                            scanline[(scanline_row * curr_display_width + x) * 2 + 1] = obj->rectangle.color & 0xFF;
                        }
                    }
                    break;
                }
                case OBJECT_TEXT: {
                    // clang-format off
                    int16_t y_start = MAX(0, obj->y > scan_y_px_start ? obj->y : scan_y_px_start);
                    int16_t y_end = MIN(obj->y + FONT_HEIGHT * obj->text.font_size < scan_y_px_end ? obj->y + FONT_HEIGHT * obj->text.font_size : scan_y_px_end, curr_display_height);
                    // clang-format on

                    // Render strip of the text
                    for (int16_t y = y_start; y < y_end; y++) {
                        // How many px of the text have been rendered so far
                        int16_t rendered_px = y - obj->y;
                        // Which row of the glyph we are on
                        int16_t glyph_row = rendered_px / obj->text.font_size;
                        // Which row of the scanline we are on
                        int16_t scanline_row = y - scan_y_px_start;

                        // Iterate over each character
                        char *text = obj->text.text;

                        int16_t char_counter = 0;

                        while (*text) {
                            char c = *text;

                            // Guard against unsupported characters in the font
                            if (c < FONT_CHAR_START || c > FONT_CHAR_END) {
                                text++;
                                continue;
                            }

                            // 10x16 font stores each glyph row as 2 bytes (little-endian)
                            uint16_t char_row_data = (uint16_t)font[c - FONT_CHAR_START][glyph_row * 2] |
                                                     ((uint16_t)font[c - FONT_CHAR_START][glyph_row * 2 + 1] << 8);

                            // Iterate over each bit
                            for (uint8_t bit = 0; bit < FONT_WIDTH; bit++) {
                                int16_t character_x_base =
                                    obj->x + char_counter + ((FONT_WIDTH - bit - 1) * obj->text.font_size);

                                // Use top FONT_WIDTH bits from the 16-bit row container.
                                if ((char_row_data >> (15 - bit)) & 0x1) {

                                    for (int16_t s = 0; s < obj->text.font_size; s++) {
                                        int16_t x = character_x_base + s;

                                        if (x < 0 || x >= curr_display_width) {
                                            continue;
                                        }

                                        int16_t pos = (scanline_row * curr_display_width + x) * 2;

                                        scanline[pos] = obj->text.color >> 8;
                                        scanline[pos + 1] = obj->text.color & 0xFF;
                                    }
                                }
                            }

                            char_counter += FONT_WIDTH * obj->text.font_size + obj->text.font_size;
                            text++;
                        }
                    }

                    break;
                }
                case OBJECT_CIRCLE: {
                    int16_t y_start = MAX(obj->y - obj->circle.radius, scan_y_px_start);
                    int16_t y_end = MIN(obj->y + obj->circle.radius, scan_y_px_end);

                    for (int16_t y = y_start; y < y_end; y++) {
                        int16_t scanline_row = y - scan_y_px_start;

                        // Get distance from center of circle
                        int16_t dy = y - obj->y;

                        // Get x distance using circle equation
                        int16_t dx = sqrt(obj->circle.radius * obj->circle.radius - dy * dy);

                        int16_t x_start = MAX(0, obj->x - dx);
                        int16_t x_end = MIN(MAX(obj->x + dx, 0), curr_display_width);

                        for (int16_t x = x_start; x < x_end; x++) {
                            int16_t pos = (scanline_row * curr_display_width + x) * 2;

                            scanline[pos] = obj->circle.color >> 8;
                            scanline[pos + 1] = obj->circle.color & 0xFF;
                        }
                    }

                    break;
                }
                case OBJECT_LINE: {
                    int16_t y0 = obj->y;
                    int16_t y1 = obj->line.y_end;
                    int16_t x_start = obj->x;
                    int16_t y_start = y0;
                    int16_t x_end = obj->line.x_end;
                    int16_t y_end = y1;

                    if (y0 != y1) {
                        int16_t line_min_y = MIN(y0, y1);
                        int16_t line_max_y = MAX(y0, y1);

                        // Clip current segment to this scanline band and display bounds.
                        y_start = MAX(MAX(line_min_y, scan_y_px_start), 0);
                        y_end = MIN(MIN(line_max_y, scan_y_px_end), curr_display_height);

                        if (y_start > y_end) {
                            break;
                        }

                        float t_start = get_blend_factor(y_start, y0, y1);
                        float t_end = get_blend_factor(y_end, y0, y1);

                        x_start = lerp_i(obj->x, obj->line.x_end, t_start);
                        x_end = lerp_i(obj->x, obj->line.x_end, t_end);
                    }

                    if (obj->line.stroke > 1) {
                        // Calculate perpendicular vector (-dy, dx)
                        int16_t dx = abs(x_end - x_start);
                        int16_t dy = abs(y_end - y_start);

                        // Normalize
                        int len = MAX(abs(dx), abs(dy)); // Approximate length is good enough here
                        if (len == 0) {
                            scanline_render_line(scan_y_px_start, x_start, y_start, x_end, y_end, obj->line.color);
                            break;
                        }
                        int px = (-dy * 256) / len; // Fixed point scaling to avoid floats
                        int py = (dx * 256) / len;

                        int half = obj->line.stroke / 2;

                        for (int i = -half; i <= half; i++) {
                            // Add half (128) to round
                            // Scale back by dividing by 256
                            int ox = (px * i + 128) >> 8;
                            int oy = (py * i + 128) >> 8;

                            scanline_render_line(
                                scan_y_px_start, x_start + ox, y_start + oy, x_end + ox, y_end + oy, obj->line.color);
                        }
                    } else {
                        scanline_render_line(scan_y_px_start, x_start, y_start, x_end, y_end, obj->line.color);
                    }

                    break;
                }
                case OBJECT_TRIANGLE: {
                    // clang-format off
                    int16_t y_start = MIN(MAX(obj->triangle.min_y > scan_y_px_start ? obj->triangle.min_y : scan_y_px_start, 0), curr_display_height);
                    int16_t y_end = MIN(MAX(obj->triangle.max_y < scan_y_px_end ? obj->triangle.max_y : scan_y_px_end, 0), curr_display_height);

                    int16_t x_start = MAX(obj->triangle.min_x, 0);
                    int16_t x_end = MIN(obj->triangle.max_x, curr_display_width);
                    // clang-format on

                    int32_t signed_area =
                        edge_function(obj->triangle.vertices[0], obj->triangle.vertices[1], obj->triangle.vertices[2]);

                    if (signed_area == 0) {
                        break;
                    }

                    vector2_t p = {x_start, y_start};

                    int32_t abp_row = edge_function(obj->triangle.vertices[0], obj->triangle.vertices[1], p);
                    int32_t bcp_row = edge_function(obj->triangle.vertices[1], obj->triangle.vertices[2], p);
                    int32_t cap_row = edge_function(obj->triangle.vertices[2], obj->triangle.vertices[0], p);

                    for (int16_t y = y_start; y < y_end; y++) {
                        int16_t scanline_row = y - scan_y_px_start;

                        int32_t abp = abp_row;
                        int32_t bcp = bcp_row;
                        int32_t cap = cap_row;

                        for (int16_t x = x_start; x < x_end; x++) {

                            // If signed area is positive, all edge functions should be positive and vice versa
                            bool positive = signed_area > 0 && abp >= 0 && bcp >= 0 && cap >= 0;
                            bool negative = signed_area < 0 && abp <= 0 && bcp <= 0 && cap <= 0;

                            if (positive || negative) {
                                scanline[(scanline_row * curr_display_width + x) * 2] = obj->triangle.color >> 8;
                                scanline[(scanline_row * curr_display_width + x) * 2 + 1] = obj->triangle.color & 0xFF;
                            }

                            abp += obj->triangle.ab.step_x;
                            bcp += obj->triangle.bc.step_x;
                            cap += obj->triangle.ca.step_x;
                        }

                        abp_row += obj->triangle.ab.step_y;
                        bcp_row += obj->triangle.bc.step_y;
                        cap_row += obj->triangle.ca.step_y;
                    }

                    break;
                }
                default:
                    platform_log(PLATFORM_LOG_LEVEL_INFO, TAG, "Can't render %d", obj->type);
                    break;
                }

            } else {
                // Once we find an empty slot, we can skip the rest since they will be empty too
                break;
            }
        }

        display_config->set_window(0, scan_y_px_start, curr_display_width - 1, scan_y_px_end - 1);
        spi_display->spi_send_data(scanline, curr_display_width * scanline_height * 2);
    }
}

void scanline_init(display_config_t *config) {
    display_config = config;

    scanline_gfx.error = PLATFORM_OK;

    free(scanline);
    scanline = malloc(display_config->get_width() * TFT_SCANLINE_HEIGHT * 2);

    free(scanline_residents);
    scanline_residents = calloc(get_scanline_count(), sizeof(*scanline_residents));

    if (scanline == NULL || scanline_residents == NULL) {
        scanline_gfx.error = PLATFORM_NO_MEM;
    }
}

gfx_t scanline_gfx = {
    .gfx_init = scanline_init,
    .error = PLATFORM_OK,
    .gfx_global_update = scanline_global_update,
    .gfx_reset = gfx_reset,
    .gfx_add_rectangle = scanline_add_rectangle,
    .gfx_add_circle = scanline_add_circle,
    .gfx_add_text = scanline_add_text,
    .gfx_add_line = scanline_add_line,
    .gfx_render = scanline_render_scene,
    .gfx_update_scene_object = scanline_update_scene_object,
};