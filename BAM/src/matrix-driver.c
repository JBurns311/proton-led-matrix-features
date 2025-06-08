#include <pico/platform/compiler.h>
#include <string.h>
#include "hardware/irq.h"
#include "matrix-driver.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"

static uint dma_chan;
static volatile uint8_t bam_idx = 0;
static volatile uint8_t render_period = 1;
static volatile uint8_t render_period_ct = 0;

static Frame dual_frame_buf[2][BAM_WIDTH] = { 0 };
static volatile uint8_t active_buf = 0;
static volatile uint8_t draw_buf = 1;

static volatile bool render_flag = false;

static void matrix_dma_irq_handler() {
    dma_hw->ints0 = 1u << dma_chan;

    render_period_ct++;
    if (render_period_ct >= render_period) {
        render_period_ct = 0;
        if (bam_idx >= BAM_WIDTH - 1) {
            bam_idx = 0;
            render_period = 1;

            // if a render is ready, swap the active and draw buffers
            if (render_flag) {
                uint8_t temp = active_buf;
                active_buf = draw_buf;
                draw_buf = temp;

                render_flag = false; // mark it safe to unblock render pipeline
            }
        } else {
            bam_idx++;
            render_period *= 2;
        }
    }

    dma_channel_set_trans_count(dma_chan, sizeof(Frame), false);
    dma_channel_set_read_addr(dma_chan, &(dual_frame_buf[active_buf][bam_idx]), true);
}

void matrix_driver_init() {
    // spi clock and data
    spi_init(spi1, MATRIX_BAUD);
    gpio_set_function(SPI1_TX, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_SCLK, GPIO_FUNC_SPI);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // LE config
    const uint slice_num = pwm_gpio_to_slice_num(EXTERNAL_CLK_PIN);
    const uint bits_per_packet = sizeof(Layer) * 8;
    gpio_init(SPI1_LE);
    gpio_set_function(EXTERNAL_CLK_PIN, GPIO_FUNC_PWM);
    gpio_set_function(SPI1_LE, GPIO_FUNC_PWM);
    pwm_set_clkdiv_mode(slice_num, PWM_DIV_B_FALLING);
    pwm_set_wrap(slice_num, bits_per_packet - 1);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(SPI1_LE), bits_per_packet - 1);
    pwm_set_output_polarity(slice_num, true, false);
    pwm_set_counter(slice_num, 1);
    pwm_set_enabled(slice_num, true);

    // init dual_frame_buf's layer ids
    for (int32_t i = 0; i < 2; i++) {
        for (int32_t j = 0; j < BAM_WIDTH; j++) {
            for (int32_t k = 0; k <= MAX_MATRIX_POS; k++) {
                dual_frame_buf[i][j].layers[k].id = MAX_MATRIX_POS - k;
            }
        }
    }


    for (int dist = 0; dist < 15; dist++) {
        for (int x = 0; x <= MAX_MATRIX_POS && x <= dist; x++) {
            for (int y = 0; y <= MAX_MATRIX_POS && y <= dist; y++) {
                if (x + y == dist) {
                    for (int z = 0; z <= MAX_MATRIX_POS; z++) {
                        matrix_draw_voxel(x, y, z, color(dist + 1, 0, 0));
                    }
                }
            }
        }
    }

    uint8_t temp = active_buf;
    active_buf = draw_buf;
    draw_buf = temp;

    // DMA config
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config dma_config = dma_channel_get_default_config(dma_chan);

    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_dreq(&dma_config, spi_get_dreq(spi1, true));
    dma_channel_configure(dma_chan, &dma_config, &spi_get_hw(spi1)->dr, &(dual_frame_buf[active_buf][bam_idx]), sizeof(Frame), false);
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, matrix_dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    dma_channel_start(dma_chan);
}

void matrix_render() {
    render_flag = true;

    while (render_flag) {
        tight_loop_contents();
    }
}


// draw api
Color color(uint8_t r, uint8_t g, uint8_t b) {
    return (Color) {
        .bgr0 = (b & 0b1) << 2 | (g & 0b1) << 1 | (r * 0b1),
        .bgr1 = (b & 0b10) << 2 | (g & 0b10) << 1 | (r * 0b10),
        .bgr2 = (b & 0b100) << 2 | (g & 0b100) << 1 | (r * 0b100),
        .bgr3 = (b & 0b1000) << 2 | (g & 0b1000) << 1 | (r * 0b1000)
    };
}

void matrix_draw_voxel(uint8_t x, uint8_t y, uint8_t z, Color c) {
    hard_assert(x <= MAX_MATRIX_POS);
    hard_assert(y <= MAX_MATRIX_POS);
    hard_assert(z <= MAX_MATRIX_POS);

    const uint8_t row_idx = y * 3;
    const uint8_t lshift_bit_idx = 29 - x * 3;
    const uint32_t reset_mask = ~(0b111 << lshift_bit_idx);

    // reset voxel location values and set new values
    //  unrolled loop for hopefully better performance
    // TODO: DOUBLE CHECK THIS!!
    *(uint32_t*)&(dual_frame_buf[draw_buf][0].layers[z].data[y * 3]) &= reset_mask;
    *(uint32_t*)&(dual_frame_buf[draw_buf][0].layers[z].data[row_idx]) |= c.bgr0  << lshift_bit_idx;
    *(uint32_t*)&(dual_frame_buf[draw_buf][1].layers[z].data[y * 3]) &= reset_mask;
    *(uint32_t*)&(dual_frame_buf[draw_buf][1].layers[z].data[row_idx]) |= c.bgr1  << lshift_bit_idx;
    *(uint32_t*)&(dual_frame_buf[draw_buf][2].layers[z].data[y * 3]) &= reset_mask;
    *(uint32_t*)&(dual_frame_buf[draw_buf][2].layers[z].data[row_idx]) |= c.bgr2  << lshift_bit_idx;
    *(uint32_t*)&(dual_frame_buf[draw_buf][3].layers[z].data[y * 3]) &= reset_mask;
    *(uint32_t*)&(dual_frame_buf[draw_buf][3].layers[z].data[row_idx]) |= c.bgr3  << lshift_bit_idx;
}

void matrix_clear_frame() {
    for (int i = 0; i < BAM_WIDTH; i++) {
        for (int j = 0; j <= MAX_MATRIX_POS; j++) {
            memset(dual_frame_buf[draw_buf][i].layers[j].data, 0, sizeof(dual_frame_buf[draw_buf][i].layers[j].data));
        }
    }
}
