#include <stdio.h>
#include "pico/stdio.h"
#include "matrix-driver.h"
#include "pico/time.h"

int main() {
    stdio_init_all();
    matrix_driver_init();

    while (true) {
        /*matrix_clear_frame();*/
        /**/
        /*for (int dist = 0; dist < 15; dist++) {*/
        /*    for (int x = 0; x <= MAX_MATRIX_POS && x <= dist; x++) {*/
        /*        for (int y = 0; y <= MAX_MATRIX_POS && y <= dist; y++) {*/
        /*            if (x + y == dist) {*/
        /*                for (int z = 0; z <= MAX_MATRIX_POS; z++) {*/
        /*                    matrix_draw_voxel(x, y, z, color(dist + 1, 0, 0));*/
        /*                }*/
        /*            }*/
        /*        }*/
        /*    }*/
        /*}*/
        /**/
        /*matrix_render();*/
        tight_loop_contents();
    }

    return 0;
}
