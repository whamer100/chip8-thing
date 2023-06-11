#include "chip8.h"
#include "macros.h"
#include <string.h>
#include <stdio.h>
#include "utils.h"
#include <raylib.h>

unsigned char font_rom[80] = {
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
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

int key_map[16] = {
    'X', '1', '2', '3',
    'Q', 'W', 'E', 'A',
    'S', 'D', 'Z', 'C',
    '4', 'R', 'F', 'V'
    /* Keymap:
     * 1234
     * QWER
     * ASDF
     * ZXCV
     * */
};

void PollKeyboard(chip8_ctx* ctx)
{
    PollInputEvents();
    for (int i = 0; i < 16; ++i) {
        ctx->keys[i] = IsKeyDown(key_map[i]);
    }
}


uint16_t chip8_fetch(chip8_ctx* ctx)
{
    if (ctx->PC >= 4095 || ctx->PC + 1 >= 4095)
    {
        printf("[CHIP8::FETCH] Program Counter exceeding the memory capacity! (Spinning CPU in place...)\n");
        return 0x1FFF;
    }
    uint16_t inst = (ctx->mem[ctx->PC] << 8 | ctx->mem[ctx->PC + 1]);
    // memcpy(&inst, ctx->mem + ctx->PC, sizeof(inst));
    ctx->PC += 2;
    return inst;
}

void chip8_step(chip8_ctx* ctx)
{
    // there's probably a better way to do this
    // but who gives a shit
    uint16_t instruction = chip8_fetch(ctx); // its decode time
    // for all types of instructions
    // CNNN
    uint8_t category = extract_bits(instruction, 0xF000);
    uint16_t NNN = extract_bits(instruction, 0x0FFF);
    // CXNN
    uint8_t X = extract_bits(instruction, 0x0F00);
    uint8_t NN = extract_bits(instruction, 0x00FF);
    // CXYN
    uint8_t Y = extract_bits(instruction, 0x00F0);
    uint8_t N = extract_bits(instruction, 0x000F);
    // i hope you arent allergic to switch statements
    switch (category)
    {
        case 0: {
            if (NN == 0xE0)
                chip8_CLS(ctx);  // clear screen
            else if (NN == 0xEE)
                chip8_RET(ctx);  // return from subroutine
            else
                dout("Something has gone horribly wrong :D_:\n");
        } break;
        case 0x1: chip8_JMP(ctx, NNN); break; // jump to immediate address
        case 0x2: chip8_CALL_NNN(ctx, NNN); break; // call subroutine
        case 0x3: chip8_SE_VX_NN(ctx, X, NN); break; // skip next inst if VX == NN
        case 0x4: chip8_SNE_VX_NN(ctx, X, NN); break; // skip next inst if VX != NN
        case 0x5: chip8_SE_VX_VY(ctx, X, Y); break; // skip next inst if VX == VY
        case 0x6: chip8_LD_VX_NN(ctx, X, NN); break; // load immediate byte into VX
        case 0x7: chip8_ADD_VX_NN(ctx, X, NN); break; // add NN to VX -> VX
        case 0x8: {
            switch (N) {
                case 0x0: chip8_LD_VX_VY(ctx, X, Y); break; // load VY -> VX
                case 0x1: chip8_OR_VX_VY(ctx, X, Y); break; // VX | VY -> VX
                case 0x2: chip8_AND_VX_VY(ctx, X, Y); break; // VX & VY -> VX
                case 0x3: chip8_XOR_VX_VY(ctx, X, Y); break; // VX ^ VY -> VX
                case 0x4: chip8_ADD_VX_VY(ctx, X, Y); break; // VX + VY -> VX [overflow flag]
                case 0x5: chip8_SUB_VX_VY(ctx, X, Y); break; // VX - VY -> VX [borrow flag]
                case 0x6: chip8_SHR_VX_(ctx, X, Y); break; // VX >> 1 -> VX [shift into flag]
                case 0x7: chip8_SUBN_VX_VY(ctx, X, Y); break; // VX - VY -> VX [no borrow flag]
                case 0xE: chip8_SHL_VX_(ctx, X, Y); break; // VX << 1 -> VX [shift into flag]
                default:
                    dout("Something has gone horribly wrong :HOW:\n");
                    break;
            }
        } break;
        case 0x9: chip8_SNE_VX_VY(ctx, X, Y); break; // skip next inst if VX != VY
        case 0xA: chip8_LD_I_NNN(ctx, NNN); break; // NNN -> I
        case 0xB: chip8_JMP_V0_NNN(ctx, NNN); break; // jump to NNN + V0
        case 0xC: chip8_RND_VX_NN(ctx, X, NN); break; // random number -> VX
        case 0xD: chip8_DRW_VX_VY_N(ctx, X, Y, N); break; // draw
        case 0xE: {
            if (NN == 0x9E)
                chip8_SKP_VX(ctx, X); // skip next inst if value of VX is pressed
            else if (NN == 0xA1)
                chip8_SKNP_VX(ctx, X); // skip next inst if value of VX is NOT pressed
            else
                dout("Something has gone horribly wrong :monkaEyes:\n");
        } break;
        case 0xF: {
            switch (NN) {
                case 0x07: chip8_LD_VX_DT(ctx, X); break; // delay timer -> VX
                case 0x0A: chip8_LD_VX_K(ctx, X); break; // wait for key press -> VX
                case 0x15: chip8_LD_DT_VX(ctx, X); break; // VX -> DT
                case 0x18: chip8_LD_ST_VX(ctx, X); break; // VX -> ST
                case 0x1E: chip8_ADD_I_VX(ctx, X); break; // I + VX -> I
                case 0x29: chip8_LD_F_VX(ctx, X); break; // VX * 5 -> I [font]
                case 0x33: chip8_LD_B_VX(ctx, X); break; // split digits (0xAE (174) -> `1, 7 ,4`) -> [I]..[I+2]
                case 0x55: chip8_LD_I_VX(ctx, X); break; // dump registers to [I+0]..[I+X]
                case 0x65: chip8_LD_VX_I(ctx, X); break; // load registers from [I+0]..[I+X]
                default:
                    dout("Something has gone horribly wrong :monkaS:\n");
                    break;
            }
        } break;
        default:
            dout("Something has gone horribly wrong :monkaW:\n");
            break;
    }
}

void chip8_CLS(chip8_ctx* ctx) // 00E0
{
    dout("[CHIP8::RUN] (PC: %u) CLS [Clearing screen]\n", ctx->PC);
    ImageClearBackground(ctx->canvas, BLACK);
}

// dout("[CHIP8::RUN] (PC: %u)  %u []", ctx->PC);

void chip8_RET(chip8_ctx* ctx) // 00EE
{
    dout("[CHIP8::RUN] (PC: %u) RET [Return from subroutine]\n", ctx->PC);
    ctx->SP--;
    ctx->PC = ctx->stack[ctx->SP];
}

void chip8_JMP(chip8_ctx* ctx, uint16_t NNN) // 1NNN
{
    dout("[CHIP8::RUN] (PC: %u) JMP %u [Jump to address]\n", ctx->PC, NNN);
    ctx->PC = NNN;
}

void chip8_CALL_NNN(chip8_ctx* ctx, uint16_t NNN) // 2NNN
{
    dout("[CHIP8::RUN] (PC: %u) CALL %u [Call subroutine]\n", ctx->PC, NNN);
    ctx->stack[ctx->SP] = ctx->PC;
    ctx->SP++;
    ctx->PC = NNN;
}

void chip8_SE_VX_NN(chip8_ctx* ctx, uint8_t X, uint8_t NN) // 3XNN
{
    dout("[CHIP8::RUN] (PC: %u) SE V%u, %u [skip next inst if VX == NN]\n", ctx->PC, X, NN);
    if (ctx->reg.V[X] == NN)
        ctx->PC += 2;
}

void chip8_SNE_VX_NN(chip8_ctx* ctx, uint8_t X, uint8_t NN) // 4XNN
{
    dout("[CHIP8::RUN] (PC: %u) SNE V%u, %u [skip next inst if VX != NN]\n", ctx->PC, X, NN);
    if (ctx->reg.V[X] != NN)
        ctx->PC += 2;
}

void chip8_SE_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y) // 5XY0
{
    dout("[CHIP8::RUN] (PC: %u) SE V%u, V%u [skip next inst if VX == VY]\n", ctx->PC, X, Y);
    if (ctx->reg.V[X] == ctx->reg.V[Y])
        ctx->PC += 2;
}

void chip8_LD_VX_NN(chip8_ctx* ctx, uint8_t X, uint8_t NN) // 6XNN
{
    dout("[CHIP8::RUN] (PC: %u) LD V%u, %u [load immediate byte into VX]\n", ctx->PC, X, NN);
    ctx->reg.V[X] = NN;
}

void chip8_ADD_VX_NN(chip8_ctx* ctx, uint8_t X, uint8_t NN) // 7XNN
{
    dout("[CHIP8::RUN] (PC: %u) ADD V%u, %u [add NN to VX -> VX]\n", ctx->PC, X, NN);
    ctx->reg.V[X] += NN;
}

void chip8_LD_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y) // 8XY0
{
    dout("[CHIP8::RUN] (PC: %u) LD V%u, V%u [load VY -> VX]\n", ctx->PC, X, Y);
    ctx->reg.V[X] = ctx->reg.V[Y];
}

void chip8_OR_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y) // 8XY1
{
    dout("[CHIP8::RUN] (PC: %u) OR V%u, V%u [VX | VY -> VX]\n", ctx->PC, X, Y);
    ctx->reg.V[X] |= ctx->reg.V[Y];
}

void chip8_AND_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y) // 8XY2
{
    dout("[CHIP8::RUN] (PC: %u) AND V%u, V%u [VX & VY -> VX]\n", ctx->PC, X, Y);
    ctx->reg.V[X] &= ctx->reg.V[Y];
}

void chip8_XOR_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y) // 8XY3
{
    dout("[CHIP8::RUN] (PC: %u) XOR V%u, V%u [VX ^ VY -> VX]\n", ctx->PC, X, Y);
    ctx->reg.V[X] ^= ctx->reg.V[Y];
}

void chip8_ADD_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y) // 8XY4
{
    dout("[CHIP8::RUN] (PC: %u) ADD V%u, V%u [VX + VY -> VX [overflow flag]]\n", ctx->PC, X, Y);
    bool flag = (ctx->reg.V[X] + ctx->reg.V[Y] > 0xFF) ? 1 : 0;
    ctx->reg.V[X] += ctx->reg.V[Y];
    ctx->reg.V[0xF] = flag;
}

void chip8_SUB_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y) // 8XY5
{
    dout("[CHIP8::RUN] (PC: %u) SUB V%u, V%u [VX - VY -> VX [borrow flag]]\n", ctx->PC, X, Y);
    bool flag = (ctx->reg.V[X] > ctx->reg.V[Y]) ? 1 : 0;
    ctx->reg.V[X] -= ctx->reg.V[Y];
    ctx->reg.V[0xF] = flag;
}

void chip8_SHR_VX_(chip8_ctx* ctx, uint8_t X, uint8_t Y) // 8XY6
{
    dout("[CHIP8::RUN] (PC: %u) SHR V%u {, V%u} [VX >> 1 -> VX [shift into flag]]\n", ctx->PC, X, Y);
#ifdef QUIRK_SHIFT
    ctx->reg.V[X] = ctx->reg.V[Y];
#endif
    bool flag = ctx->reg.V[X] & 1;
    ctx->reg.V[X] >>= 1;
    ctx->reg.V[0xF] = flag;
}

void chip8_SUBN_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y) // 8XY7
{
    dout("[CHIP8::RUN] (PC: %u) SUBN V%u, V%u [VX - VY -> VX [no borrow flag]]\n", ctx->PC, X, Y);
    bool flag = (ctx->reg.V[Y] > ctx->reg.V[X]) ? 1 : 0;
    ctx->reg.V[X] = ctx->reg.V[Y] - ctx->reg.V[X];
    ctx->reg.V[0xF] = flag;
}

void chip8_SHL_VX_(chip8_ctx* ctx, uint8_t X, uint8_t Y) // 8XYE
{
    dout("[CHIP8::RUN] (PC: %u) SHL V%u {, V%u} [VX << 1 -> VX [shift into flag]]\n", ctx->PC, X, Y);
#ifdef QUIRK_SHIFT
    ctx->reg.V[X] = ctx->reg.V[Y];
#endif
    bool flag = ctx->reg.V[X] & 0x80;
    ctx->reg.V[X] <<= 1;
    ctx->reg.V[0xF] = flag;
}

void chip8_SNE_VX_VY(chip8_ctx* ctx, uint8_t X, uint8_t Y) // 9XY0
{
    dout("[CHIP8::RUN] (PC: %u) SNE V%u, V%u [skip next inst if VX != VY]\n", ctx->PC, X, Y);
    if (ctx->reg.V[X] != ctx->reg.V[Y])
        ctx->PC += 2;
}

void chip8_LD_I_NNN(chip8_ctx* ctx, uint16_t NNN) // ANNN
{
    dout("[CHIP8::RUN] (PC: %u) LD I, %u [NNN -> I]\n", ctx->PC, NNN);
    ctx->reg.I = NNN;
}

void chip8_JMP_V0_NNN(chip8_ctx* ctx, uint16_t NNN) // BNNN
{
    dout("[CHIP8::RUN] (PC: %u) JMP %u, V0 [jump to NNN + V0]\n", ctx->PC, NNN);
    ctx->PC = ctx->reg.V[0] + NNN;
}

void chip8_RND_VX_NN(chip8_ctx* ctx, uint8_t X, uint8_t NN) // CXNN
{
    dout("[CHIP8::RUN] (PC: %u) RND V%u, %u [random number %% NN -> VX]\n", ctx->PC, X, NN);
    ctx->reg.V[X] = GetRandomValue(0, 0x7FFF) & NN;
}

void chip8_DRW_VX_VY_N(chip8_ctx* ctx, uint8_t X, uint8_t Y, uint8_t N) // DXYN
{
    dout("[CHIP8::RUN] (PC: %u) DRAW V%u, V%u, %u [draw]\n", ctx->PC, X, Y, N);
    uint8_t x = ctx->reg.V[X] % DISPLAY_WIDTH;
    uint8_t y = ctx->reg.V[Y] % DISPLAY_WIDTH; // DISPLAY_HEIGHT;

    for (int row = 0; row < N; ++row) {
        uint8_t bits = ctx->mem[ctx->reg.I + row];
        uint8_t cy = (y + row) % DISPLAY_HEIGHT;
        for (int col = 0; col < 8; ++col) {
            uint8_t cx = (x + col) % DISPLAY_WIDTH;
            Color cur_col = GetImageColor(*ctx->canvas, cx, cy);
            int cur_col_int = ColorToInt(cur_col);
            uint8_t state = bits & (0x01 << (7 - col));
            if (state > 0) {
                if (cur_col_int == ctx->fg) {
                    ImageDrawPixel(ctx->canvas, cx, cy, ctx->background);
                    ctx->reg.V[0xF] = 1;
                } else {
                    ImageDrawPixel(ctx->canvas, cx, cy, ctx->foreground);
                }
            }
            if (cx == DISPLAY_WIDTH - 1) break;
        }
        if (cy == DISPLAY_HEIGHT - 1) break;
    }
    ctx->should_update = true;
}

void chip8_SKP_VX(chip8_ctx* ctx, uint8_t X) // EX9E
{
    dout("[CHIP8::RUN] (PC: %u) SKP V%u [skip next inst if value of VX is pressed]\n", ctx->PC, X);
    PollKeyboard(ctx);
    if (ctx->keys[ctx->reg.V[X]])
        ctx->PC += 2;
}

void chip8_SKNP_VX(chip8_ctx* ctx, uint8_t X) // EXA1
{
    dout("[CHIP8::RUN] (PC: %u) SKNP V%u [skip next inst if value of VX is NOT pressed]\n", ctx->PC, X);
    PollKeyboard(ctx);
    if (!ctx->keys[ctx->reg.V[X]])
        ctx->PC += 2;
}

void chip8_LD_VX_DT(chip8_ctx* ctx, uint8_t X) // FX07
{
    dout("[CHIP8::RUN] (PC: %u) LD V%u, DT [delay timer -> VX]\n", ctx->PC, X);
    ctx->reg.V[X] = ctx->reg.DT;
}

void chip8_LD_VX_K(chip8_ctx* ctx, uint8_t X) // FX0A
{
    dout("[CHIP8::RUN] (PC: %u) LD V%u, K [wait for key press -> VX]\n", ctx->PC, X);
    if (!ctx->keys[ctx->reg.V[X]])
        ctx->PC -= 2; // this is kind of a hack, but it will work LOL
}

void chip8_LD_DT_VX(chip8_ctx* ctx, uint8_t X) // FX15
{
    dout("[CHIP8::RUN] (PC: %u) LD DT, V%u [VX -> DT]\n", ctx->PC, X);
    ctx->reg.DT = ctx->reg.V[X];
}

void chip8_LD_ST_VX(chip8_ctx* ctx, uint8_t X) // FX18
{
    dout("[CHIP8::RUN] (PC: %u) LD ST, V%u [VX -> ST]\n", ctx->PC, X);
    ctx->reg.ST = ctx->reg.V[X];
}

void chip8_ADD_I_VX(chip8_ctx* ctx, uint8_t X) // FX1E
{
    dout("[CHIP8::RUN] (PC: %u) ADD I, V%u [I + VX -> I]\n", ctx->PC, X);
    ctx->reg.I += ctx->reg.V[X];
}

void chip8_LD_F_VX(chip8_ctx* ctx, uint8_t X) // FX29
{
    dout("[CHIP8::RUN] (PC: %u) LD F, V%u [VX * 5 -> I [font]]\n", ctx->PC, X);
    ctx->reg.I = ctx->reg.V[X] * 5;
}

void chip8_LD_B_VX(chip8_ctx* ctx, uint8_t X) // FX33
{
    dout("[CHIP8::RUN] (PC: %u) LD B, V%u [split digits (0xAE (174) -> `1, 7 ,4`) -> [I]..[I+2]]\n", ctx->PC, X);
    uint8_t x = ctx->reg.V[X];
    uint8_t h = x / 100;
    uint8_t t = (x - h * 100) / 10;
    uint8_t o = x - h * 100 - t * 10;

    ctx->mem[ctx->reg.I] = h;
    ctx->mem[ctx->reg.I + 1] = t;
    ctx->mem[ctx->reg.I + 2] = o;
}

void chip8_LD_I_VX(chip8_ctx* ctx, uint8_t X) // FX55
{
    dout("[CHIP8::RUN] (PC: %u) LD [I], V%u [dump registers to [I+0]..[I+X]]\n", ctx->PC, X);
    for (int v = 0; v <= X; ++v)
        ctx->mem[ctx->reg.I + v] = ctx->reg.V[v];
}

void chip8_LD_VX_I(chip8_ctx* ctx, uint8_t X) // FX65
{
    dout("[CHIP8::RUN] (PC: %u) LD V%u, [I] [load registers from [I+0]..[I+X]]\n", ctx->PC, X);
    for (int v = 0; v <= X; ++v)
        ctx->reg.V[v] = ctx->mem[ctx->reg.I + v];
}