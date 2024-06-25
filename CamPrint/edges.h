#pragma once
#include <Arduino.h>

void edges(uint8_t* frame, int width, int height) {
    const int kernel[][3] = {{-1, -1, -1}, {-1, 9, -1}, {-1, -1, -1}};

    uint8_t* copy = (uint8_t*)ps_malloc(width * height);
    if (!copy) return;
    memcpy(copy, frame, width * height);

    for (int x = 1; x < width - 1; x++) {
        for (int y = 1; y < height - 1; y++) {
            int sum = 0;
            for (int kx = -1; kx <= 1; kx++) {
                for (int ky = -1; ky <= 1; ky++) {
                    int val = copy[(x + kx) + (y + ky) * width];
                    sum += kernel[ky + 1][kx + 1] * val;
                }
            }
            sum = (sum < 0) ? 0 : (sum > 255 ? 255 : sum);
            frame[x + y * width] = sum;
        }
    }

    free(copy);
}