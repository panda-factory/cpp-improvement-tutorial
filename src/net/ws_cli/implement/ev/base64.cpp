//
// Created by admin on 2021/2/16.
//

#include "base64.h"
#include <iostream>
#include <cctype>
namespace {
const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";


inline bool IsBase64(const unsigned char c) {
    return (std::isalnum(c) || (c == '+') || (c == '/'));
}
}

std::string EncodeBase64(const unsigned char* bytes_to_encode, unsigned int len) {
    int modulus = len % 3;
    int pad = ((modulus&1)<<1) + ((modulus&2)>>1); // modulus 2 pad 1 bit, and 1 pad 2 bit, but 0 gives 0.
    std::string res;

    size_t i = 0;
    //! go into this loop when len >= 3.
    for (; i <= len - 3 && len >= 3; i += 3) {
        unsigned char BYTE0 = bytes_to_encode[i];
        unsigned char BYTE1 = bytes_to_encode[i + 1];
        unsigned char BYTE2 = bytes_to_encode[i + 2];
        res += base64_chars[BYTE0 >> 2];
        res += base64_chars[((0x3&BYTE0) << 4) + (BYTE1 >> 4)];
        res += base64_chars[((0x0f&BYTE1) << 2) + (BYTE2 >> 6)];
        res += base64_chars[0x3f&BYTE2];
    }

    if(pad == 2) {
        unsigned char BYTE0 = bytes_to_encode[i];
        res += base64_chars[BYTE0 >> 2];
        res += base64_chars[((0x3&BYTE0) << 4)];
        res += '=';
        res += '=';
    } else if(pad == 1) {
        unsigned char BYTE0 = bytes_to_encode[i];
        unsigned char BYTE1 = bytes_to_encode[i + 1];
        res += base64_chars[BYTE0 >> 2];
        res += base64_chars[((0x3&BYTE0) << 4) + (BYTE1 >> 4)];
        res += base64_chars[((0x0f&BYTE1) << 2)];
        res += '=';
    }

    return std::move(res);

}

std::string DecodeBase64(const std::string& base64Str) {
    int len = base64Str.size();
    std::string res;
    int pad = 0;


    if (base64Str[ len-1 ]=='=' ) ++pad;
    if (base64Str[ len-2 ]=='=' ) ++pad;

    int charNo = 0;
    for (; charNo <= len - 4 - pad; charNo += 4 )
    {
        uint8_t BYTE0 = base64_chars.find(base64Str[charNo]);
        uint8_t BYTE1 = base64_chars.find(base64Str[charNo + 1]);
        uint8_t BYTE2 = base64_chars.find(base64Str[charNo + 2]);
        uint8_t BYTE3 = base64_chars.find(base64Str[charNo + 3]);

        res += (BYTE0<<2) | (BYTE1>>4) ;
        res += (BYTE1<<4) | (BYTE2>>2) ;
        res += (BYTE2<<6) | (BYTE3) ;
    }

    if (pad == 1) {
        uint8_t BYTE0 = base64_chars.find(base64Str[charNo]);
        uint8_t BYTE1 = base64_chars.find(base64Str[charNo + 1]);
        uint8_t BYTE2 = base64_chars.find(base64Str[charNo + 2]);

        res += (BYTE0<<2) | (BYTE1>>4) ;
        res += (BYTE1<<4) | (BYTE2>>2) ;
    } else if (pad == 2) {
        uint8_t BYTE0 = base64_chars.find(base64Str[charNo]);
        uint8_t BYTE1 = base64_chars.find(base64Str[charNo + 1]);

        res += (BYTE0<<2) | (BYTE1>>4) ;
    }

    return res;
}