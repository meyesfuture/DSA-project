CC ?= gcc

# Auto-detect target architecture from compiler (32-bit vs 64-bit MinGW)
IS_64BIT := $(shell $(CC) -dumpmachine | findstr /i "64")

ifeq ($(strip $(IS_64BIT)),)
    SDL_ARCH = i686-w64-mingw32
else
    SDL_ARCH = x86_64-w64-mingw32
endif

# Check if ../SDL2-2.30.10 exists locally; otherwise fallback to system SDL2
ifneq ($(wildcard ../SDL2-2.30.10/$(SDL_ARCH)),)
    SDL_DIR = ../SDL2-2.30.10/$(SDL_ARCH)
    CFLAGS ?= -Wall -Wextra -O2 -I$(SDL_DIR)/include
    LDFLAGS ?= -L$(SDL_DIR)/lib -lmingw32 -lSDL2main -lSDL2
    COPY_DLL = cmd /c if exist $(subst /,\,$(SDL_DIR))\bin\SDL2.dll copy /Y $(subst /,\,$(SDL_DIR))\bin\SDL2.dll SDL2.dll >nul
else
    CFLAGS ?= -Wall -Wextra -O2
    LDFLAGS ?= -lmingw32 -lSDL2main -lSDL2
    COPY_DLL =
endif

TARGET = chip8.exe
SOURCES = main.c emulator_decode_ins.c emulator_setup.c input_handling.c monitor.c opcode_stats.c
HEADERS = chip8.h opcode_stats.h

all: $(TARGET) copy_dll

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $(TARGET)

copy_dll:
	$(COPY_DLL)

clean:
	cmd /c if exist $(TARGET) del /f /q $(TARGET)
