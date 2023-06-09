#ifndef CHIP8_THING_CHIP8_H
#define CHIP8_THING_CHIP8_H

#include <stdint.h>
#include "raylib.h"

typedef struct chip8_ctx_s chip8_ctx;
struct chip8_ctx_s;
typedef struct chip8_reg_s chip8_reg;
struct chip8_reg_s;

// emulator registers
struct chip8_reg_s {
    uint8_t V[16];
    uint16_t I;
};
/*
 * uint8_t V : size 16 -> 8 bit registers from 0 through F
 * uint16_t I -> 16 bit register
 * */

// emulator context
struct chip8_ctx_s {
    uint8_t mem[0xFFF];
    uint16_t stack[16];
    chip8_reg reg;
    uint16_t PC;
    uint16_t SP;
    float ft;

    bool program_loaded;
    char* program_path;

    Image* canvas;
    Texture2D* tex;
};
/*
 * uint8_t mem[] : size 4096 -> emulator memory
 * uint16_t stack[] : size 16 -> stack memory
 * chip8_reg reg -> emulator registers
 * uint16_t PC -> program counter
 * uint8_t SP -> stack pointer
 * */

unsigned char font_rom[80];

#endif //CHIP8_THING_CHIP8_H
