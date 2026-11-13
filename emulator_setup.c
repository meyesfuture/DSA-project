#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "chip8.h"

// Standard 8x5 CHIP-8 fontset (0-F)
static const uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void console_init(struct chip8* console) {
    if (!console) return;

    memset(console, 0, sizeof(struct chip8));
    console->PC = 0x200;
    console->draw_flag = true;

    // Load standard fontset into RAM starting at 0x050
    memcpy(&console->RAM[0x050], fontset, sizeof(fontset));
}

void console_load(const char* cartridge_title, struct chip8* console) {
    if (!cartridge_title || !console) return;

    FILE* cartridge = fopen(cartridge_title, "rb");
    if (!cartridge) {
        // Try fallback to look in same directory if path was simple filename
        char fallback_path[512];
        snprintf(fallback_path, sizeof(fallback_path), "./%s", cartridge_title);
        cartridge = fopen(fallback_path, "rb");
    }

    if (!cartridge) {
        fprintf(stderr, "Error: Could not open ROM file '%s'\n", cartridge_title);
        exit(1);
    }

    fseek(cartridge, 0, SEEK_END);
    long cartridge_size = ftell(cartridge);
    rewind(cartridge);

    if (cartridge_size <= 0) {
        fprintf(stderr, "Error: ROM file '%s' is empty.\n", cartridge_title);
        fclose(cartridge);
        exit(1);
    }

    const unsigned long max_size = 4096 - 0x200;
    if ((unsigned long)cartridge_size > max_size) {
        fprintf(stderr, "Error: ROM (%ld bytes) exceeds available memory (%lu bytes)!\n",
                cartridge_size, max_size);
        fclose(cartridge);
        exit(1);
    }

    size_t bytes_read = fread(console->RAM + 0x200, 1, cartridge_size, cartridge);
    fclose(cartridge);

    if (bytes_read != (size_t)cartridge_size) {
        fprintf(stderr, "Warning: Expected %ld bytes, but read %lu bytes from '%s'\n",
                cartridge_size, (unsigned long)bytes_read, cartridge_title);
    } else {
        printf("Loaded ROM '%s' (%lu bytes) into RAM at 0x200.\n", cartridge_title, (unsigned long)bytes_read);
    }
}

uint16_t console_fetch(struct chip8* console) {
    if (!console) return 0x0000;

    if (console->PC >= 4095) {
        return 0x0000; // Safe NOP if PC is out of RAM bounds
    }

    uint16_t opcode = ((uint16_t)console->RAM[console->PC] << 8) | console->RAM[console->PC + 1];
    console->PC += 2;
    return opcode;
}