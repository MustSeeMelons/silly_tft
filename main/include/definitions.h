#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

typedef enum
{
    // NOTE: these are ili9341 values
    TFT_Portrait = 0x48,         // 0 Degrees
    TFT_Landscape = 0x28,        // 90 Degrees
    TFT_Portrait_Flipped = 0x88, // 180 Degrees
    TFT_Landscape_Flipped = 0xE8 // 270 Degrees
} tft_rotation_t;

#endif