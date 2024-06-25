#pragma once
#include <Arduino.h>

void dither(uint8_t* frame, int width, int height) {
    uint32_t len = width * height;
    int16_t* buf = (int16_t*)ps_malloc(len * sizeof(int16_t));
    if (!buf) return;

    for (uint32_t i = 0; i < len; i++) {
        buf[i] = frame[i];
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            int col = (buf[idx] < 128) ? 0 : 255;
            int err = buf[idx] - col;
            buf[idx] = col;
            if (x + 1 < width) buf[idx + 1] += (err * 7) >> 4;
            if (y + 1 == height) continue;

            if (x > 0) buf[idx + width - 1] += (err * 3) >> 4;
            buf[idx + width] += (err * 5) >> 4;
            if (x + 1 < width) buf[idx + width + 1] += (err * 1) >> 4;
        }
    }

    for (uint32_t i = 0; i < len; i++) {
        frame[i] = buf[i];
    }
    free(buf);
}