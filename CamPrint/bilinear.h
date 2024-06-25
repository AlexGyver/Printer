#pragma once
#include <Arduino.h>
// https://chao-ji.github.io/jekyll/update/2018/07/19/BilinearResize.html

void bilinear_interp(uint8_t *input, int input_width, int input_height,
                     uint8_t *output, int output_width, int output_height) {
    float x_ratio = (input_width - 1) / (output_width - 1);
    float y_ratio = (input_height - 1) / (output_height - 1);

    for (int i = 0; i < output_height; i++) {
        for (int j = 0; j < output_width; j++) {
            int x_l = floor(x_ratio * j);
            int y_l = floor(y_ratio * i);
            int x_h = ceil(x_ratio * j);
            int y_h = ceil(y_ratio * i);

            float x_wei = (x_ratio * j) - x_l;
            float y_wei = (y_ratio * i) - y_l;

            float a = input[y_l * input_width + x_l];
            float b = input[y_l * input_width + x_h];
            float c = input[y_h * input_width + x_l];
            float d = input[y_h * input_width + x_h];

            uint8_t val = a * (1.0 - x_wei) * (1.0 - y_wei) +
                          b * x_wei * (1.0 - y_wei) +
                          c * y_wei * (1.0 - x_wei) +
                          d * x_wei * y_wei;

            output[i * output_width + j] = val;
        }
    }
}