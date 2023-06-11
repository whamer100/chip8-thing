#ifndef CHIP8_THING_MACROS_H
#define CHIP8_THING_MACROS_H

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define WINDOW_SCALE 10

#define CYCLES_PER_SECOND 4

#define QUIRK_SHIFT 1

// log every instruction being executed
// #define DEBUG_LOGGING

#ifdef DEBUG_LOGGING
#define dout(...) printf(__VA_ARGS__)
#endif
#ifndef DEBUG_LOGGING
#define dout(...)
#endif

#endif //CHIP8_THING_MACROS_H
