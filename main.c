#include "raylib.h"
#include "chip8.h"
#include "nfd.h"
#include "string.h"
#include "macros.h"
#include <stdbool.h>
#include <stdio.h>
#include <malloc.h>

// typedef const char* string;
// typedef char* string;

void update(float dt, chip8_ctx* ctx);
void draw(chip8_ctx* ctx);

void init_update(chip8_ctx* ctx);
void init_draw();

void prompt_file(char* *program_path);

int main(void)
{
    // Initialization
    const int screenWidth = DISPLAY_WIDTH * WINDOW_SCALE;
    const int screenHeight = DISPLAY_HEIGHT * WINDOW_SCALE;

    InitWindow(screenWidth, screenHeight, "chip8 emulator");

    SetTargetFPS(240);

    // SetRandomSeed(0);


    chip8_ctx ctx = { 0 };

    ctx.background = BLACK;
    ctx.bg = ColorToInt(ctx.background);
    ctx.foreground = WHITE;
    ctx.fg = ColorToInt(ctx.foreground);

    ctx.program_path = calloc(1, 32767);  // windows path limit moment

    Image chip8_canvas = GenImageColor(DISPLAY_WIDTH, DISPLAY_HEIGHT, ctx.background);
    Texture2D chip8_tex = LoadTextureFromImage(chip8_canvas);

    ctx.canvas = &chip8_canvas;
    ctx.tex = &chip8_tex;


    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // i wish there was a cleaner way to do this
        // and i will not be using a switch statement or anything
        // [ammend] oh past me, i wish you didnt say this
        if (!ctx.program_loaded)
        {
            init_update(&ctx);
            init_draw();
            continue;
        }
        float dt = GetFrameTime();
        update(dt, &ctx);
        draw(&ctx);
    }

    // De-Initialization
    free(ctx.program_path);
    CloseWindow();        // Close window and OpenGL context

    return 0;
}

void update(float dt, chip8_ctx* ctx)
{
    // i hate numbers
    //ctx->ft += dt;
    //if (ctx->ft < 1/60.f) return;
    //ctx->ft -= 1/60.f;

    // TODO: CPU instructions [in progress]
    // TODO: input handling [done]
    // TODO: sound
    // TODO: custom colors [kinda]
    // TODO: whatever else i need

    // ctx->reg.V[0]++;
    // ctx->reg.V[0] %= 64;

    // ImageClearBackground(ctx->canvas, RED);
    // ImageDrawPixel(ctx->canvas, ctx->reg.V[0], 0, WHITE);
    ctx->should_update = false;
    for (int cycles = 0; cycles < CYCLES_PER_SECOND; ++cycles) {
        if (ctx->reg.DT > 0) ctx->reg.DT--;
        if (ctx->reg.ST > 0) ctx->reg.ST--;
        chip8_step(ctx);
    }
}

void draw(chip8_ctx* ctx)
{
    BeginDrawing();
    // ClearBackground(BLACK);
    if (ctx->should_update) {
        UpdateTexture(*ctx->tex, ctx->canvas->data);
    }

    Vector2 pos = {0, 0};
    DrawTextureEx(*ctx->tex, pos, 0, WINDOW_SCALE, WHITE);

    EndDrawing();
}

void init_update(chip8_ctx* ctx)
{
    if (IsFileDropped())
    {
        printf("[LOAD::CHIP8] File dropped.\n");
        FilePathList files = LoadDroppedFiles();

        if (files.count > 0)
        {
            TextCopy(ctx->program_path, files.paths[0]);
            printf("[LOAD::CHIP8] File recieved. (%s)\n", ctx->program_path);
        }

        UnloadDroppedFiles(files);
    }
    if (IsMouseButtonPressed(0))
    {
        prompt_file(&ctx->program_path);
    }
    if (strlen(ctx->program_path) > 0)
    {
        ctx->program_loaded = true;
        unsigned char* rom_data;
        unsigned int rom_size;

        rom_data = LoadFileData(ctx->program_path, &rom_size);

        if (rom_size > (0xDFF))
        {
            // TODO: error here
            ctx->program_loaded = false;
            printf("[LOAD::CHIP8::FATAL] ROM SIZE TOO LARGE!. (%d / 4096 bytes)\n", rom_size);
        }
        else
        {
            /// INITIALIZATION ROUTINE ///
            printf("[LOAD::CHIP8] Loading rom. (%d bytes)\n", rom_size);
            memset(ctx->mem, 0, 0xFFF); // ensure memory is cleared
            memcpy(ctx->mem, font_rom, sizeof(font_rom)); // copy font rom data to 0x000
            memcpy(ctx->mem + 0x200, rom_data, rom_size); // copy rom
            ctx->PC = 0x200; // entrypoint
        }

        UnloadFileData(rom_data);
    }
}

void init_draw()
{
    BeginDrawing();
    ClearBackground(BLACK);

    DrawText("Click here or drag a program to load it!", 110, 130, 20, LIGHTGRAY);
    DrawText("(Click file dialogs are slow, idk why)", 175, 170, 16, LIGHTGRAY);

    EndDrawing();
}

// why does this cause the program to lag so hard
// it shouldn't, but it does! TODO: why?
void prompt_file(char* *program_path)
{
    NFD_Init();
    printf("[LOAD::CHIP8] Prompting user for file...\n");
    nfdfilteritem_t filterItem[1] = { { "CHIP-8 Program File", "ch8" }};
    nfdchar_t* outPath;

    nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
    if (result == NFD_OKAY)
    {
        printf("[LOAD::CHIP8] File recieved. (%s)\n", outPath);
        TextCopy(*program_path, outPath);
        NFD_FreePath(outPath);
    }
    else if (result == NFD_CANCEL)
    {
        printf("[LOAD::CHIP8] User canceled prompt.\n");
    }
    else
    {
        printf("[LOAD::CHIP8::FATAL] Error: %s\n", NFD_GetError());
    }

    NFD_Quit();
}