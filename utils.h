#ifndef CHIP8_THING_UTILS_H
#define CHIP8_THING_UTILS_H

#include <stdint.h>

inline uint16_t clz(uint16_t x);

uint16_t extract_bits(uint16_t val, uint16_t mask);

#endif //CHIP8_THING_UTILS_H
