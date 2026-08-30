#ifndef DEFS_H_
#define DEFS_H_

typedef enum
{
    TFT_Portrait = 0,
    TFT_Landscape = 1,
    TFT_Portrait_Flipped = 2,
    TFT_Landscape_Flipped = 3
} tft_rotation_t;

typedef enum
{
    PLATFORM_OK = 0,
    PLATFORM_NO_MEM = 1
} platform_err_t;

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#endif