#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "chip8.h"

void console_decode(struct chip8* console, uint16_t opcode) {
    if (!console) return;

    uint8_t N1  = (opcode >> 12) & 0x0F;
    uint8_t N2  = (opcode >> 8)  & 0x0F; // Vx
    uint8_t N3  = (opcode >> 4)  & 0x0F; // Vy
    uint8_t N4  = opcode & 0x0F;         // N
    uint8_t NN  = opcode & 0x00FF;       // 8-bit immediate
    uint16_t NNN = opcode & 0x0FFF;      // 12-bit address

    switch (N1) {
        case 0x0:
            switch (NNN) {
                case 0x00E0:
                    // 00E0: Clear screen
                    CLS(console);
                    break;

                case 0x00EE:
                    // 00EE: Return from subroutine
                    if (console->SP > 0) {
                        console->SP--;
                        console->PC = console->stack[console->SP];
                    }
                    break;

                default:
                    // 0NNN: RCA 1802 machine code call (ignored on modern emulators)
                    break;
            }
            break;

        case 0x1:
            // 1NNN: Jump to address NNN
            console->PC = NNN;
            break;

        case 0x2:
            // 2NNN: Call subroutine at NNN
            if (console->SP < 16) {
                console->stack[console->SP] = console->PC;
                console->SP++;
                console->PC = NNN;
            }
            break;

        case 0x3:
            // 3XNN: Skip next instruction if Vx == NN
            if (console->V[N2] == NN) {
                console->PC += 2;
            }
            break;

        case 0x4:
            // 4XNN: Skip next instruction if Vx != NN
            if (console->V[N2] != NN) {
                console->PC += 2;
            }
            break;

        case 0x5:
            // 5XY0: Skip next instruction if Vx == Vy
            if (N4 == 0) {
                if (console->V[N2] == console->V[N3]) {
                    console->PC += 2;
                }
            }
            break;

        case 0x6:
            // 6XNN: Set Vx = NN
            console->V[N2] = NN;
            break;

        case 0x7:
            // 7XNN: Set Vx = Vx + NN (carry flag unchanged)
            console->V[N2] += NN;
            break;

        case 0x8:
            switch (N4) {
                case 0x0:
                    // 8XY0: Set Vx = Vy
                    console->V[N2] = console->V[N3];
                    break;

                case 0x1:
                    // 8XY1: Set Vx = Vx | Vy, reset VF = 0
                    console->V[N2] |= console->V[N3];
                    console->V[0xF] = 0;
                    break;

                case 0x2:
                    // 8XY2: Set Vx = Vx & Vy, reset VF = 0
                    console->V[N2] &= console->V[N3];
                    console->V[0xF] = 0;
                    break;

                case 0x3:
                    // 8XY3: Set Vx = Vx ^ Vy, reset VF = 0
                    console->V[N2] ^= console->V[N3];
                    console->V[0xF] = 0;
                    break;

                case 0x4: {
                    // 8XY4: Set Vx = Vx + Vy, VF = carry
                    uint16_t sum = (uint16_t)console->V[N2] + (uint16_t)console->V[N3];
                    uint8_t carry = (sum > 255) ? 1 : 0;
                    console->V[N2] = (uint8_t)(sum & 0xFF);
                    console->V[0xF] = carry;
                    break;
                }

                case 0x5: {
                    // 8XY5: Set Vx = Vx - Vy, VF = NOT borrow (1 if Vx >= Vy, 0 otherwise)
                    uint8_t not_borrow = (console->V[N2] >= console->V[N3]) ? 1 : 0;
                    console->V[N2] = console->V[N2] - console->V[N3];
                    console->V[0xF] = not_borrow;
                    break;
                }

                case 0x6: {
                    // 8XY6: Shift right Vx by 1, VF = LSB shifted out
                    uint8_t lsb = console->V[N2] & 0x01;
                    console->V[N2] >>= 1;
                    console->V[0xF] = lsb;
                    break;
                }

                case 0x7: {
                    // 8XY7: Set Vx = Vy - Vx, VF = NOT borrow (1 if Vy >= Vx, 0 otherwise)
                    uint8_t not_borrow = (console->V[N3] >= console->V[N2]) ? 1 : 0;
                    console->V[N2] = console->V[N3] - console->V[N2];
                    console->V[0xF] = not_borrow;
                    break;
                }

                case 0xE: {
                    // 8XYE: Shift left Vx by 1, VF = MSB shifted out
                    uint8_t msb = (console->V[N2] >> 7) & 0x01;
                    console->V[N2] <<= 1;
                    console->V[0xF] = msb;
                    break;
                }

                default:
                    break;
            }
            break;

        case 0x9:
            // 9XY0: Skip next instruction if Vx != Vy
            if (N4 == 0) {
                if (console->V[N2] != console->V[N3]) {
                    console->PC += 2;
                }
            }
            break;

        case 0xA:
            // ANNN: Set I = NNN
            console->I = NNN;
            break;

        case 0xB:
            // BNNN: Jump to NNN + V0
            console->PC = NNN + console->V[0];
            break;

        case 0xC: {
            // CXNN: Set Vx = rand() & NN
            uint8_t random_byte = (uint8_t)(rand() % 256);
            console->V[N2] = random_byte & NN;
            break;
        }

        case 0xD:
            // DXYN: Draw sprite at (Vx, Vy) with height N
            display(console, N2, N3, N4);
            break;

        case 0xE: {
            uint8_t key = console->V[N2] & 0x0F;
            switch (NN) {
                case 0x9E:
                    // EX9E: Skip next instruction if key in Vx is pressed
                    if (console->keys[key]) {
                        console->PC += 2;
                    }
                    break;

                case 0xA1:
                    // EXA1: Skip next instruction if key in Vx is NOT pressed
                    if (!console->keys[key]) {
                        console->PC += 2;
                    }
                    break;

                default:
                    break;
            }
            break;
        }

        case 0xF:
            switch (NN) {
                case 0x07:
                    // FX07: Set Vx = delay_timer
                    console->V[N2] = console->delay_timer;
                    break;

                case 0x15:
                    // FX15: Set delay_timer = Vx
                    console->delay_timer = console->V[N2];
                    break;

                case 0x18:
                    // FX18: Set sound_timer = Vx
                    console->sound_timer = console->V[N2];
                    break;

                case 0x1E:
                    // FX1E: Set I = I + Vx (wrap within 12-bit address space)
                    console->I = (console->I + console->V[N2]) & 0x0FFF;
                    break;

                case 0x29:
                    // FX29: Set I = location of font sprite for character in Vx (0-F)
                    console->I = 0x050 + ((console->V[N2] & 0x0F) * 5);
                    break;

                case 0x33: {
                    // FX33: Store BCD representation of Vx at I, I+1, I+2
                    if (console->I + 2 < 4096) {
                        uint8_t val = console->V[N2];
                        console->RAM[console->I]     = val / 100;
                        console->RAM[console->I + 1] = (val / 10) % 10;
                        console->RAM[console->I + 2] = val % 10;
                    }
                    break;
                }

                case 0x55:
                    // FX55: Store registers V0 through Vx in memory starting at I
                    for (int i = 0; i <= N2 && (console->I + i) < 4096; i++) {
                        console->RAM[console->I + i] = console->V[i];
                    }
                    break;

                case 0x65:
                    // FX65: Read registers V0 through Vx from memory starting at I
                    for (int i = 0; i <= N2 && (console->I + i) < 4096; i++) {
                        console->V[i] = console->RAM[console->I + i];
                    }
                    break;

                case 0x0A: {
                    // FX0A: Wait for a key press, store key value in Vx
                    bool key_pressed = false;
                    for (int i = 0; i < 16; i++) {
                        if (console->keys[i]) {
                            console->V[N2] = (uint8_t)i;
                            key_pressed = true;
                            break;
                        }
                    }
                    if (!key_pressed) {
                        console->PC -= 2; // Keep waiting by re-executing this instruction
                    }
                    break;
                }

                default:
                    break;
            }
            break;

        default:
            break;
    }
}