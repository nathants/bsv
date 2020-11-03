// from: https://github.com/jandrewrogers/AquaHash/blob/f07b524fa605f0218c6eae6a33586ea158cb01a9/aquahash.h
// Copyright 2018 J. Andrew Rogers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <smmintrin.h>
#include <wmmintrin.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define max_input SIZE_MAX - 1
// sentinel to prevent double finalization
#define finalized max_input + 1

typedef struct AquaHash_s {
    // INCREMENTAL CONSTRUCTION STATE
    // 4 x 128-bit hashing lanes
    __m128i block[4];
    // input block buffer
    __m128i input[4];
    // initialization vector
    __m128i initialize;
    // cumulative input bytes
    size_t input_bytes;
    // result
    uint64_t result[2];
} AquaHash_t;

// Reference implementation of AquaHash small key algorithm
static void AquaHash_smallkey(AquaHash_t *state, const uint8_t * key, const size_t bytes) {
    __m128i initialize = _mm_setzero_si128();
    assert(bytes <= max_input);
    __m128i hash = initialize;
    // bulk hashing loop -- 128-bit block size
    const __m128i * ptr128 = (const __m128i *)(key);
    if (bytes / sizeof(hash)) {
        __m128i temp = _mm_set_epi64x(0xa11202c9b468bea1, 0xd75157a01452495b);
        for (int32_t i = 0; i < bytes / sizeof(hash); ++i) {
            __m128i b = _mm_loadu_si128(ptr128++);
            hash = _mm_aesenc_si128(hash, b);
            temp = _mm_aesenc_si128(temp, b);
        }
        hash = _mm_aesenc_si128(hash, temp);
    }
    // AES sub-block processor
    const uint8_t * ptr8 = (const uint8_t *)(ptr128);
    if (bytes & 8) {
        __m128i b = _mm_set_epi64x(*(const uint64_t*)(ptr8), 0xa11202c9b468bea1);
        hash = _mm_xor_si128(hash, b);
        ptr8 += 8;
    }
    if (bytes & 4) {
        __m128i b = _mm_set_epi32(0xb1293b33, 0x05418592, *(const int32_t*)(ptr8), 0xd210d232);
        hash = _mm_xor_si128(hash, b);
        ptr8 += 4;
    }
    if (bytes & 2) {
        __m128i b = _mm_set_epi16(0xbd3d, 0xc2b7, 0xb87c, 0x4715, 0x6a6c, 0x9527, *(const uint16_t*)(ptr8), 0xac2e);
        hash = _mm_xor_si128(hash, b);
        ptr8 += 2;
    }
    if (bytes & 1) {
        __m128i b = _mm_set_epi8(0xcc, 0x96, 0xed, 0x16, 0x74, 0xea, 0xaa, 0x03, 0x1e, 0x86, 0x3f, 0x24, 0xb2, 0xa8, *(const uint8_t*)(ptr8), 0x31);
        hash = _mm_xor_si128(hash, b);
    }
    // this algorithm construction requires no less than three AES rounds to finalize
    hash = _mm_aesenc_si128(hash, _mm_set_epi64x(0x8e51ef21fabb4522, 0xe43d7a0656954b6c));
    hash = _mm_aesenc_si128(hash, _mm_set_epi64x(0x56082007c71ab18f, 0x76435569a03af7fa));
    hash = _mm_aesenc_si128(hash, _mm_set_epi64x(0xd2600de7157abc68, 0x6339e901c3031efb));
    memcpy(state->result, &hash, sizeof(state->result));
}

// Reference implementation of AquaHash large key algorithm
static __m128i AquaHash_largekey(AquaHash_t *state, const uint8_t * key, const size_t bytes) {
    __m128i initialize = _mm_setzero_si128();
    assert(bytes <= max_input);
    // initialize 4 x 128-bit hashing lanes, for a 512-bit block size
    __m128i block[4];
    block[0] = _mm_xor_si128(initialize, _mm_set_epi64x(0xa11202c9b468bea1, 0xd75157a01452495b));
    block[1] = _mm_xor_si128(initialize, _mm_set_epi64x(0xb1293b3305418592, 0xd210d232c6429b69));
    block[2] = _mm_xor_si128(initialize, _mm_set_epi64x(0xbd3dc2b7b87c4715, 0x6a6c9527ac2e0e4e));
    block[3] = _mm_xor_si128(initialize, _mm_set_epi64x(0xcc96ed1674eaaa03, 0x1e863f24b2a8316a));
    // bulk hashing loop -- 512-bit block size
    const __m128i * ptr128 = (const __m128i *)(key);
    for (size_t block_counter = 0; block_counter < bytes / sizeof(block); block_counter++) {
        block[0] = _mm_aesenc_si128(block[0], _mm_loadu_si128(ptr128++));
        block[1] = _mm_aesenc_si128(block[1], _mm_loadu_si128(ptr128++));
        block[2] = _mm_aesenc_si128(block[2], _mm_loadu_si128(ptr128++));
        block[3] = _mm_aesenc_si128(block[3], _mm_loadu_si128(ptr128++));
    }
    // process remaining AES blocks
    if (bytes & 32) {
        block[0] = _mm_aesenc_si128(block[0], _mm_loadu_si128(ptr128++));
        block[1] = _mm_aesenc_si128(block[1], _mm_loadu_si128(ptr128++));
    }
    if (bytes & 16) {
        block[2] = _mm_aesenc_si128(block[2], _mm_loadu_si128(ptr128++));
    }
    // AES sub-block processor
    const uint8_t * ptr8 = (const uint8_t *)(ptr128);
    if (bytes & 8) {
        __m128i b = _mm_set_epi64x(*(const uint64_t*)(ptr8), 0xa11202c9b468bea1);
        block[3] = _mm_aesenc_si128(block[3], b);
        ptr8 += 8;
    }
    if (bytes & 4) {
        __m128i b = _mm_set_epi32(0xb1293b33, 0x05418592, *(const uint32_t*)(ptr8), 0xd210d232);
        block[0] = _mm_aesenc_si128(block[0], b);
        ptr8 += 4;
    }
    if (bytes & 2) {
        __m128i b = _mm_set_epi16(0xbd3d, 0xc2b7, 0xb87c, 0x4715, 0x6a6c, 0x9527, *(const uint16_t*)(ptr8), 0xac2e);
        block[1] = _mm_aesenc_si128(block[1], b);
        ptr8 += 2;
    }
    if (bytes & 1) {
        __m128i b = _mm_set_epi8(0xcc, 0x96, 0xed, 0x16, 0x74, 0xea, 0xaa, 0x03, 0x1e, 0x86, 0x3f, 0x24, 0xb2, 0xa8,*ptr8, 0x31);
        block[2] = _mm_aesenc_si128(block[2], b);
    }
    // indirectly mix hashing lanes
    const __m128i mix  = _mm_xor_si128(_mm_xor_si128(block[0], block[1]), _mm_xor_si128(block[2], block[3]));
    block[0] = _mm_aesenc_si128(block[0], mix);
    block[1] = _mm_aesenc_si128(block[1], mix);
    block[2] = _mm_aesenc_si128(block[2], mix);
    block[3] = _mm_aesenc_si128(block[3], mix);
    // reduction from 512-bit block size to 128-bit hash
    __m128i hash = _mm_aesenc_si128(_mm_aesenc_si128(block[0],block[1]), _mm_aesenc_si128(block[2], block[3]));
    // this algorithm construction requires no less than one round to finalize
    hash = _mm_aesenc_si128(hash, _mm_set_epi64x(0x8e51ef21fabb4522, 0xe43d7a0656954b6c));
    memcpy(state->result, &hash, sizeof(state->result));
}

static void AquaHash_init(AquaHash_t *state) {
        state->initialize = _mm_setzero_si128();
        state->input_bytes = 0;
        state->block[0] = _mm_xor_si128(state->initialize, _mm_set_epi64x(0xa11202c9b468bea1, 0xd75157a01452495b));
        state->block[1] = _mm_xor_si128(state->initialize, _mm_set_epi64x(0xb1293b3305418592, 0xd210d232c6429b69));
        state->block[2] = _mm_xor_si128(state->initialize, _mm_set_epi64x(0xbd3dc2b7b87c4715, 0x6a6c9527ac2e0e4e));
        state->block[3] = _mm_xor_si128(state->initialize, _mm_set_epi64x(0xcc96ed1674eaaa03, 0x1e863f24b2a8316a));
}

static void AquaHash_update(AquaHash_t *state, const uint8_t * key, size_t bytes) {
    assert(state->input_bytes != finalized);
    assert(bytes <= max_input && max_input - state->input_bytes >= bytes);
    if (bytes == 0)
        return;
    // input buffer may be partially filled
    if (state->input_bytes % sizeof(state->input)) {
        // pointer to first unused byte in state->input buffer
        uint8_t * ptr8 = (uint8_t *)(state->input) + (state->input_bytes % sizeof(state->input));
        // compute initial copy size from key to state->input buffer
        size_t copy_size = sizeof(state->input) - (state->input_bytes % sizeof(state->input));
        if (copy_size > bytes) copy_size = bytes;
        // append new key bytes to state->input buffer
        memcpy(ptr8, key, copy_size);
        state->input_bytes += copy_size;
        bytes       -= copy_size;
        // state->input buffer not filled by update
        if (state->input_bytes % sizeof(state->input))
            return;
        // update key pointer to first byte not in the state->input buffer
        key += copy_size;
        // hash state->input buffer
        state->block[0] = _mm_aesenc_si128(state->block[0], state->input[0]);
        state->block[1] = _mm_aesenc_si128(state->block[1], state->input[1]);
        state->block[2] = _mm_aesenc_si128(state->block[2], state->input[2]);
        state->block[3] = _mm_aesenc_si128(state->block[3], state->input[3]);
    }
    state->input_bytes += bytes;
    // state->input buffer is empty
    const __m128i * ptr128 = (const __m128i *)(key);
    while (bytes >= sizeof(state->block)) {
        state->block[0] = _mm_aesenc_si128(state->block[0], _mm_loadu_si128(ptr128++));
        state->block[1] = _mm_aesenc_si128(state->block[1], _mm_loadu_si128(ptr128++));
        state->block[2] = _mm_aesenc_si128(state->block[2], _mm_loadu_si128(ptr128++));
        state->block[3] = _mm_aesenc_si128(state->block[3], _mm_loadu_si128(ptr128++));
        bytes -= sizeof(state->block);
    }
    // load remaining bytes into state->input buffer
    if (bytes)
        memcpy(state->input, ptr128, bytes);
}

static void AquaHash_finalize(AquaHash_t *state) {
    assert(state->input_bytes != finalized);
    // process remaining AES blocks
    if (state->input_bytes & 32) {
        state->block[0] = _mm_aesenc_si128(state->block[0], state->input[0]);
        state->block[1] = _mm_aesenc_si128(state->block[1], state->input[1]);
    }
    if (state->input_bytes & 16) {
        state->block[2] = _mm_aesenc_si128(state->block[2], state->input[2]);
    }
    // AES sub-state->block processor
    const uint8_t * ptr8 = (const uint8_t *)(&state->input[3]);
    if (state->input_bytes & 8) {
        __m128i b = _mm_set_epi64x(*(const uint64_t*)(ptr8), 0xa11202c9b468bea1);
        state->block[3] = _mm_aesenc_si128(state->block[3], b);
        ptr8 += 8;
    }
    if (state->input_bytes & 4) {
        __m128i b = _mm_set_epi32(0xb1293b33, 0x05418592, *(const uint32_t*)(ptr8), 0xd210d232);
        state->block[0] = _mm_aesenc_si128(state->block[0], b);
        ptr8 += 4;
    }
    if (state->input_bytes & 2) {
        __m128i b = _mm_set_epi16(0xbd3d, 0xc2b7, 0xb87c, 0x4715, 0x6a6c, 0x9527, *(const uint16_t*)(ptr8), 0xac2e);
        state->block[1] = _mm_aesenc_si128(state->block[1], b);
        ptr8 += 2;
    }
    if (state->input_bytes & 1) {
        __m128i b = _mm_set_epi8(0xcc, 0x96, 0xed, 0x16, 0x74, 0xea, 0xaa, 0x03,
                                 0x1e, 0x86, 0x3f, 0x24, 0xb2, 0xa8,*ptr8, 0x31);
        state->block[2] = _mm_aesenc_si128(state->block[2], b);
    }
    // indirectly mix hashing lanes
    const __m128i mix  = _mm_xor_si128(_mm_xor_si128(state->block[0], state->block[1]), _mm_xor_si128(state->block[2], state->block[3]));
    state->block[0] = _mm_aesenc_si128(state->block[0], mix);
    state->block[1] = _mm_aesenc_si128(state->block[1], mix);
    state->block[2] = _mm_aesenc_si128(state->block[2], mix);
    state->block[3] = _mm_aesenc_si128(state->block[3], mix);
    // reduction from 512-bit state->block size to 128-bit hash
    __m128i hash = _mm_aesenc_si128(_mm_aesenc_si128(state->block[0],state->block[1]), _mm_aesenc_si128(state->block[2], state->block[3]));
    // this algorithm construction requires no less than 1 round to finalize
    state->input_bytes = finalized;
    hash = _mm_aesenc_si128(hash, _mm_set_epi64x(0x8e51ef21fabb4522, 0xe43d7a0656954b6c));
    memcpy(state->result, &hash, sizeof(state->result));
}
