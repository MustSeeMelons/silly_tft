#ifndef DEFS_H_
#define DEFS_H_

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

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

#endif