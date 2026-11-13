#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "chip8.h"
#include "opcode_stats.h"

#define CHIP8_CYCLES_PER_FRAME 12 // ~720 instructions per second (12 * 60Hz)
#define AUDIO_SAMPLE_RATE 44100
#define BEEP_FREQUENCY 440

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: ./chip8 <ROM_FILE>\n");
        printf("Available sample ROMs in current folder:\n");
        printf("  ./chip8 Pong.ch8\n");
        printf("  ./chip8 space.ch8\n");
        printf("  ./chip8 tetris.ch8\n");
        return 1;
    }

    srand((unsigned int)time(NULL));

    struct chip8 console;
    struct opcode_stats stats;
    opcode_stats_init(&stats);
    console_init(&console);

    console_load(argv[1], &console);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
        opcode_stats_reset(&stats);
        return 1;
    }

    // Set nearest-neighbor scaling for sharp retro pixels
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640,
        320,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (window == NULL) {
        fprintf(stderr, "SDL window error: %s\n", SDL_GetError());
        SDL_Quit();
        opcode_stats_reset(&stats);
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        // Fallback to software renderer if accelerated is unavailable
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer == NULL) {
        fprintf(stderr, "SDL renderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        opcode_stats_reset(&stats);
        return 1;
    }

    // Maintain 64x32 aspect ratio automatically across any window size
    SDL_RenderSetLogicalSize(renderer, 64, 32);

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        64,
        32
    );

    if (texture == NULL) {
        fprintf(stderr, "SDL texture error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        opcode_stats_reset(&stats);
        return 1;
    }

    // Audio setup for CHIP-8 buzzer
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = AUDIO_SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;

    SDL_AudioDeviceID audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    int16_t beep_buffer[AUDIO_SAMPLE_RATE / 60]; // 1 frame worth of audio
    if (audio_dev > 0) {
        int half_period = (have.freq / BEEP_FREQUENCY) / 2;
        if (half_period < 1) half_period = 1;
        for (int i = 0; i < (int)(sizeof(beep_buffer) / sizeof(beep_buffer[0])); i++) {
            beep_buffer[i] = ((i / half_period) % 2 == 0) ? 3000 : -3000;
        }
    }

    printf("\n--- CHIP-8 Emulator Running: %s ---\n", argv[1]);
    printf("Controls:\n");
    printf("  Keypad:  1 2 3 4 | Q W E R | A S D F | Z X C V\n");
    printf("  Gaming:  Arrow keys (Move / Up / Down) | Space (Action/Fire)\n");
    printf("  F1:      Print Opcode Hash-Table Statistics\n");
    printf("  F2:      Reset Opcode Statistics\n");
    printf("  Esc:     Quit\n\n");

    bool running = true;
    bool stats_warning_shown = false;
    SDL_Event event;
    uint32_t video_buffer[64 * 32];

    uint32_t last_timer_tick = SDL_GetTicks();

    while (running) {
        // Poll OS and keyboard events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                if (event.key.keysym.sym == SDLK_F1) {
                    opcode_stats_print(&stats);
                } else if (event.key.keysym.sym == SDLK_F2) {
                    opcode_stats_reset(&stats);
                    stats_warning_shown = false;
                    printf("Opcode statistics reset.\n");
                }
            }

            process_input(&console, &event, &running);
        }

        if (!running) break;

        // Execute CPU cycles for this slice
        for (int i = 0; i < CHIP8_CYCLES_PER_FRAME; i++) {
            uint16_t opcode = console_fetch(&console);

            if (!opcode_stats_record(&stats, opcode) && !stats_warning_shown) {
                fprintf(stderr, "Could not record opcode statistics: out of memory.\n");
                stats_warning_shown = true;
            }

            console_decode(&console, opcode);
        }

        // 60Hz Timer Ticks
        uint32_t current_time = SDL_GetTicks();
        if (current_time - last_timer_tick >= 16) {
            last_timer_tick = current_time;

            if (console.delay_timer > 0) {
                console.delay_timer--;
            }
            if (console.sound_timer > 0) {
                console.sound_timer--;
                if (audio_dev > 0) {
                    SDL_QueueAudio(audio_dev, beep_buffer, sizeof(beep_buffer));
                    SDL_PauseAudioDevice(audio_dev, 0);
                }
            } else {
                if (audio_dev > 0) {
                    SDL_PauseAudioDevice(audio_dev, 1);
                    SDL_ClearQueuedAudio(audio_dev);
                }
            }
        }

        // Render when draw flag is set or periodically
        if (console.draw_flag) {
            console.draw_flag = false;

            for (int i = 0; i < 64 * 32; i++) {
                video_buffer[i] = (console.monitor[i] == 1) ? 0xFFFFFFFF : 0x000000FF;
            }

            SDL_UpdateTexture(texture, NULL, video_buffer, 64 * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        // Give back tiny slice of CPU to OS
        SDL_Delay(1);
    }

    if (audio_dev > 0) {
        SDL_CloseAudioDevice(audio_dev);
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    opcode_stats_reset(&stats);

    return 0;
}
