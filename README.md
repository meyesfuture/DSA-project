# CHIP-8 Emulator (with Hash Table Opcode Profiling)

A C project that emulates the classic 1970s CHIP-8 virtual machine using SDL2 for graphics and sound.

---

## How to Build & Run

### 1. Build
Open a terminal in `Chip8-emulator-main`:
```bash
mingw32-make
```
*(or simply run the pre-built `chip8.exe`)*

### 2. Run
Launch any included ROM:
```bash
.\chip8.exe Pong.ch8
.\chip8.exe space.ch8
.\chip8.exe tetris.ch8
```

---

## Controls

### Keypad Mapping
CHIP-8 uses a 16-key hex keypad (0x0 – 0xF). It is mapped to the QWERTY keyboard:

```text
CHIP-8 Keypad:          QWERTY Keyboard:
[ 1 ] [ 2 ] [ 3 ] [ C ]  -->  [ 1 ] [ 2 ] [ 3 ] [ 4 ]
[ 4 ] [ 5 ] [ 6 ] [ D ]  -->  [ Q ] [ W ] [ E ] [ R ]
[ 7 ] [ 8 ] [ 9 ] [ E ]  -->  [ A ] [ S ] [ D ] [ F ]
[ A ] [ 0 ] [ B ] [ F ]  -->  [ Z ] [ X ] [ C ] [ V ]
```

### Modern Gaming Controls
The emulator also accepts standard gamer keys:
- **Arrow Keys:**
  - **Up:** Paddle Up (Pong) / Rotate (Tetris)
  - **Down:** Paddle Down (Pong) / Soft Drop (Tetris)
  - **Left / Right:** Move Left / Right (Space Invaders & Tetris)
- **Spacebar:** Fire (Space Invaders) / Action (Tetris)
- **Esc:** Quit the emulator

---

## Opcode Statistics & Profiling

The emulator includes a hash table with separate chaining that profiles every fetched 16-bit opcode:
- Press **F1**: Print execution counts and total instructions to the terminal.
- Press **F2**: Reset statistics counter.
