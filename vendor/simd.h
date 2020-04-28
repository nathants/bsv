#ifndef SIMD_H
#define SIMD_H

#include <immintrin.h>

static inline int simd_strcmp(const char* s1, const char* s2) {
    // from: https://github.com/WojciechMula/simd-string/blob/e9f739c4b4eb953e18ccd284740f2761bf78c723/strcmp.cpp
    /* Copyright (c) 2006-2015, Wojciech Muła */
    /* All rights reserved. */
    /* Redistribution and use in source and binary forms, with or without */
    /* modification, are permitted provided that the following conditions are */
    /* met: */
    /* 1. Redistributions of source code must retain the above copyright */
    /*    notice, this list of conditions and the following disclaimer. */
    /* 2. Redistributions in binary form must reproduce the above copyright */
    /*    notice, this list of conditions and the following disclaimer in the */
    /*    documentation and/or other materials provided with the distribution. */
    /* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS */
    /* IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED */
    /* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A */
    /* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT */
    /* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, */
    /* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED */
    /* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR */
    /* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF */
    /* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING */
    /* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS */
    /* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

    /* if (s1 == s2) { */
        /* return 0; */
    /* } */
    __m128i* ptr1 = (__m128i*)((char*)(s1));
    __m128i* ptr2 = (__m128i*)((char*)(s2));
    for (/**/; /**/; ptr1++, ptr2++) {
        const __m128i a = _mm_loadu_si128(ptr1);
        const __m128i b = _mm_loadu_si128(ptr2);
        const uint8_t mode =
            _SIDD_UBYTE_OPS |
            _SIDD_CMP_EQUAL_EACH |
            _SIDD_NEGATIVE_POLARITY |
            _SIDD_LEAST_SIGNIFICANT;
        if (_mm_cmpistrc(a, b, mode)) {
            // a & b are different (not counting past-zero bytes)
            const auto idx = _mm_cmpistri(a, b, mode);
            const uint8_t b1 = ((char*)(ptr1))[idx];
            const uint8_t b2 = ((char*)(ptr2))[idx];
            if (b1 < b2) {
                return -1;
            } else if (b1 > b2) {
                return +1;
            } else {
                return 0;
            }
        } else if (_mm_cmpistrz(a, b, mode)) {
            // a & b are same, but b contains a zero byte
            break;
        }
    }
    return 0;
}

#endif
