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
    uint8_t DT; // delay timer
    uint8_t ST; // sound timer
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
    bool keys[16];
    bool should_update;
    float ft;

    bool program_loaded;
    char* program_path;

    Color background;
    Color foreground;

    // precalculate colors for comparisons
    int bg;
    int fg;

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

int key_map[16];


void chip8_step(chip8_ctx* ctx);
uint16_t chip8_fetch(chip8_ctx* ctx);

// helper functions
/*
 * call -> 0xABBB -> X
 * ret <- X & 0xF000
 * X <<= 4
 * */
uint8_t extract_nibble(uint16_t* val);


/// instructions ///
void chip8_CLS(chip8_ctx* ctx); // 00E0
void chip8_RET(chip8_ctx* ctx); // 00EE
void chip8_JMP(chip8_ctx* ctx, uint16_t NNN); // 1NNN
void chip8_CALL_NNN(chip8_ctx* ctx, uint16_t NNN); // 2NNN
void chip8_SE_VX_NN(chip8_ctx* ctx, uint8_t X, uint8_t NN); // 3XNN
void chip8_SNE_VX_NN(chip8_ctx* ctx, uint8_t X, uint8_t NN); // 4XNN
void chip8_SE_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y); // 5XY0
void chip8_LD_VX_NN(chip8_ctx* ctx, uint8_t X, uint8_t NN); // 6XNN
void chip8_ADD_VX_NN(chip8_ctx* ctx, uint8_t X, uint8_t NN); // 7XNN
void chip8_LD_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y); // 8XY0
void chip8_OR_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y); // 8XY1
void chip8_AND_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y); // 8XY2
void chip8_XOR_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y); // 8XY3
void chip8_ADD_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y); // 8XY4
void chip8_SUB_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y); // 8XY5
void chip8_SHR_VX_(chip8_ctx* ctx, uint8_t X, uint8_t Y); // 8XY6
void chip8_SUBN_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y); // 8XY7
void chip8_SHL_VX_(chip8_ctx* ctx, uint8_t X, uint8_t Y); // 8XYE
void chip8_SNE_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y); // 9XY0
void chip8_LD_I_NNN(chip8_ctx* ctx, uint16_t NNN); // ANNN
void chip8_JMP_V0_NNN(chip8_ctx* ctx, uint16_t NNN); // BNNN
void chip8_RND_VX_NN(chip8_ctx* ctx, uint8_t X, uint8_t NN); // CXNN
void chip8_DRW_VX_VY_N(chip8_ctx* ctx, uint8_t X, uint8_t Y, uint8_t N); // DXYN
void chip8_SKP_VX(chip8_ctx* ctx, uint8_t X); // EX9E
void chip8_SKNP_VX(chip8_ctx* ctx, uint8_t X); // EXA1
void chip8_LD_VX_DT(chip8_ctx* ctx, uint8_t X); // FX07
void chip8_LD_VX_K(chip8_ctx* ctx, uint8_t X); // FX0A
void chip8_LD_DT_VX(chip8_ctx* ctx, uint8_t X); // FX15
void chip8_LD_ST_VX(chip8_ctx* ctx, uint8_t X); // FX18
void chip8_ADD_I_VX(chip8_ctx* ctx, uint8_t X); // FX1E
void chip8_LD_F_VX(chip8_ctx* ctx, uint8_t X); // FX29
void chip8_LD_B_VX(chip8_ctx* ctx, uint8_t X); // FX33
void chip8_LD_I_VX(chip8_ctx* ctx, uint8_t X); // FX55
void chip8_LD_VX_I(chip8_ctx* ctx, uint8_t X); // FX65


#endif //CHIP8_THING_CHIP8_H
