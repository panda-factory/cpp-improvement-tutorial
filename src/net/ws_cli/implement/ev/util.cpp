//
// Created by admin on 2021/2/5.
//

#ifdef WIN32
#define _CRT_RAND_S
#include <cstdlib>
#endif

#include "util.h"


namespace ws {
namespace {

constexpr char* b64="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;

// maps A=>0,B=>1..
constexpr unsigned char unb64[]={
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //10
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //20
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //30
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //40
        0,   0,   0,  62,   0,   0,   0,  63,  52,  53, //50
        54,  55,  56,  57,  58,  59,  60,  61,   0,   0, //60
        0,   0,   0,   0,   0,   0,   1,   2,   3,   4, //70
        5,   6,   7,   8,   9,  10,  11,  12,  13,  14, //80
        15,  16,  17,  18,  19,  20,  21,  22,  23,  24, //90
        25,   0,   0,   0,   0,   0,   0,  26,  27,  28, //100
        29,  30,  31,  32,  33,  34,  35,  36,  37,  38, //110
        39,  40,  41,  42,  43,  44,  45,  46,  47,  48, //120
        49,  50,  51,   0,   0,   0,   0,   0,   0,   0, //130
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //140
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //150
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //160
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //170
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //180
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //190
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //200
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //210
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //220
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //230
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //240
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //250
        0,   0,   0,   0,   0,   0,
}; // This array has 255 elements
constexpr char *opcodesStr[] =
        {
                "continuation", 		// 0x0
                "text",					// 0x1
                "binary",				// 0x2
                "non control reserved",	// 0x3
                "non control reserved", // 0x4
                "non control reserved", // 0x5
                "non control reserved", // 0x6
                "non control reserved", // 0x7
                "close",				// 0x8
                "ping",					// 0x9
                "pong",					// 0xA
                "control reserved",		// 0xB
                "control reserved",		// 0xC
                "control reserved",		// 0xD
                "control reserved",		// 0xE
                "control reserved"		// 0xF
        };
} // namespace

const char* Opcode2Str(const uint8_t opcode) {
    if ((opcode >= 0) &&
        (opcode <= 0xF))
        return opcodesStr[opcode];

    return nullptr;
}

uint64_t EncodeN2H64(const uint64_t input) {
    uint64_t rval;
    uint8_t *data = (uint8_t *)&rval;

    data[0] = (uint8_t)(input >> 56);
    data[1] = (uint8_t)(input >> 48);
    data[2] = (uint8_t)(input >> 40);
    data[3] = (uint8_t)(input >> 32);
    data[4] = (uint8_t)(input >> 24);
    data[5] = (uint8_t)(input >> 16);
    data[6] = (uint8_t)(input >> 8);
    data[7] = (uint8_t)(input >> 0);

    return rval;
}

uint64_t EncodeH2N64(const uint64_t input) {
    return (EncodeN2H64(input));
}

int GetRandomMask(unsigned char *buf, size_t len) {
#ifdef _WIN32
    unsigned int tmp;

    // http://msdn.microsoft.com/en-us/library/sxtz2fa8(VS.80).aspx
    size_t i = 0;
    for (; i < len; i++) {
        rand_s(&tmp);
        buf[i] = (unsigned char)tmp;
    }
#else
    int i;
	i = read(ws->ws_base->random_fd, buf, len);
#endif

    return i;
}

} // namespace ws