#ifndef MATRIX_DRIVER_H
#define MATRIX_DRIVER_H

#include <stdint.h>

#define MATRIX_BAUD 1000000

#define SPI1_TX 15
#define SPI1_SCLK 14
#define SPI1_LE 12
#define EXTERNAL_CLK_PIN 13

#define BAM_WIDTH 4
#define MAX_MATRIX_POS 7
#define MIN_MATRIX_POS 0

#define packed __attribute__((packed))

typedef struct packed {
    uint8_t id;
    uint8_t data[24];
} Layer;

typedef struct packed {
    Layer layers[8];
} Frame;

typedef struct {
    unsigned int bgr0: 3;
    unsigned int bgr1: 3;
    unsigned int bgr2: 3;
    unsigned int bgr3: 3;
} Color;

void matrix_driver_init();

// swaps the draw frame buffer and active frame buffer
// blocks execution until current active buffer has finished current render
void matrix_render();

Color color(uint8_t r, uint8_t g, uint8_t b);

void matrix_draw_voxel(uint8_t x, uint8_t y, uint8_t z, Color c);

void matrix_clear_frame();

#endif
