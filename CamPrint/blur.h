#pragma once
#include <Arduino.h>

void blur(uint8_t* frame, int width, int height) {
    float kernel[][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
    int i = 9;
    float* f = (float*)kernel;
    while (i--) *(f++) *= 1.0 / 16;  // домножить все на 16

    uint8_t* copy = (uint8_t*)ps_malloc(width * height);
    if (!copy) return;
    memcpy(copy, frame, width * height);

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            float sum = 0;
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    sum += kernel[ky + 1][kx + 1] * copy[(x + kx) + (y + ky) * width];
                }
            }
            frame[x + y * width] = sum;
        }
    }

    free(copy);
}