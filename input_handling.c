#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "chip8.h"

// Keypad mapping:
// Standard CHIP-8 Hex Keypad (16 keys: 0x0 - 0xF) mapped to modern QWERTY:
// 1 2 3 4 -> 1 2 3 C
// Q W E R -> 4 5 6 D
// A S D F -> 7 8 9 E
// Z X C V -> A 0 B F
//
// Additionally, we map common gaming keys for convenience:
// - Arrow Keys: Left (4), Right (6), Up (1/5), Down (4/7)
// - Spacebar: Key 5 (Fire in Space Invaders, Rotate in Tetris)
// - Escape: Exit emulator
void process_input(struct chip8* console, SDL_Event* event, bool* running) {
    if (!console || !event) return;

    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE) {
        if (running) *running = false;
        return;
    }

    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        uint8_t state = (event->type == SDL_KEYDOWN) ? 1 : 0;

        switch (event->key.keysym.sym) {
            // Standard CHIP-8 16-key Hex Keypad
            case SDLK_x: console->keys[0x0] = state; break;
            case SDLK_1: console->keys[0x1] = state; break;
            case SDLK_2: console->keys[0x2] = state; break;
            case SDLK_3: console->keys[0x3] = state; break;
            case SDLK_q: console->keys[0x4] = state; break;
            case SDLK_w: console->keys[0x5] = state; break;
            case SDLK_e: console->keys[0x6] = state; break;
            case SDLK_a: console->keys[0x7] = state; break;
            case SDLK_s: console->keys[0x8] = state; break;
            case SDLK_d: console->keys[0x9] = state; break;
            case SDLK_z: console->keys[0xA] = state; break;
            case SDLK_c: console->keys[0xB] = state; break;
            case SDLK_4: console->keys[0xC] = state; break;
            case SDLK_r: console->keys[0xD] = state; break;
            case SDLK_f: console->keys[0xE] = state; break;
            case SDLK_v: console->keys[0xF] = state; break;

            // Intuitive Gaming Controls (Convenience aliases)
            case SDLK_LEFT:
                console->keys[0x4] = state; // Space Invaders / Tetris Move Left
                break;
            case SDLK_RIGHT:
                console->keys[0x6] = state; // Space Invaders / Tetris Move Right
                break;
            case SDLK_UP:
                console->keys[0x1] = state; // Pong Paddle Up
                console->keys[0x5] = state; // Tetris Rotate
                break;
            case SDLK_DOWN:
                console->keys[0x4] = state; // Pong Paddle Down
                console->keys[0x7] = state; // Tetris Soft Drop
                break;
            case SDLK_SPACE:
                console->keys[0x5] = state; // Space Invaders Fire / Tetris Rotate
                break;

            default:
                break;
        }
    }
}
