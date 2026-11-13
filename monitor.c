#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "chip8.h"

void CLS(struct chip8* console) {
    if (!console) return;
    memset(console->monitor, 0, sizeof(console->monitor));
    console->draw_flag = true;
}

void display(struct chip8* console, uint8_t N2, uint8_t N3, uint8_t N4) {
    if (!console) return;

    // Reset collision flag VF
    uint8_t collision = 0;

    // Wrap initial coordinates within display bounds (64x32)
    uint8_t x_cord = console->V[N2] % 64;
    uint8_t y_cord = console->V[N3] % 32;
    uint8_t elevation = N4;
    uint16_t sprite_address = console->I;

    for (int i = 0; i < elevation; i++) {
        // Vertical Clipping: stop drawing if we exceed the bottom edge
        if (y_cord + i >= 32) {
            break;
        }

        uint8_t sprite_byte = console->RAM[(sprite_address + i) % 4096];

        for (int j = 0; j < 8; j++) {
            // Horizontal Clipping: stop drawing if we exceed the right edge
            if (x_cord + j >= 64) {
                break;
            }

            // Check bit from left to right (0x80 down to 0x01)
            if (sprite_byte & (0x80 >> j)) {
                int screen_index = ((y_cord + i) * 64) + (x_cord + j);

                // Collision check: if the pixel on screen is already set, set collision
                if (console->monitor[screen_index] == 1) {
                    collision = 1;
                }

                // XOR the pixel onto the screen
                console->monitor[screen_index] ^= 1;
            }
        }
    }

    // Set VF to 1 if collision occurred, 0 otherwise
    console->V[0xF] = collision;
    console->draw_flag = true;
}
