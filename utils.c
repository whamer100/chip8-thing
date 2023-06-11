#include "utils.h"

inline uint16_t ctz(uint16_t x)
{
    // inspired from https://stackoverflow.com/a/45225089
    uint16_t res = 0;
    if (x) {
        if (!(x & 0x00FF)) { res +=  8; x >>=  8; }
        if (!(x & 0x000F)) { res +=  4; x >>=  4; }
        if (!(x & 0x0003)) { res +=  2; x >>=  2; }
        res += (x & 1) ^ 1;
    }

    return res;
}

uint16_t extract_bits(uint16_t val, uint16_t mask)
{
    uint16_t ret = val & mask;
    ret >>= ctz(mask);
    return ret;
}
