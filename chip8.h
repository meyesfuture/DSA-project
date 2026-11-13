#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

struct chip8 {
    uint8_t  RAM[4096];         // 4KB memory
    uint8_t  V[16];             // Registers V0 through VF
    uint16_t I;                 // 16-bit Index register
    uint16_t PC;                // Program counter (starts at 0x200)
    uint8_t  keys[16];          // Keypad state (0 = released, 1 = pressed)
    uint16_t stack[16];         // 16-level call stack
    uint8_t  SP;                // Stack pointer
    uint8_t  monitor[64 * 32];  // 64x32 monochrome display buffer
    uint8_t  delay_timer;       // 60Hz delay timer
    uint8_t  sound_timer;       // 60Hz sound timer
    bool     draw_flag;         // Set when screen needs redrawing
};

void console_init(struct chip8* console);
void console_load(const char* cartridge_title, struct chip8* console);
uint16_t console_fetch(struct chip8* console);
void console_decode(struct chip8* console, uint16_t opcode);
void display(struct chip8* console, uint8_t N2, uint8_t N3, uint8_t N4);
void CLS(struct chip8* console);
void process_input(struct chip8* console, SDL_Event* event, bool* running);

#endif // CHIP8_H
