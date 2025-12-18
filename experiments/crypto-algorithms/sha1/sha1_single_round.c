/*********************************************************************
* Filename:   sha1_single_round.c
* Based on:   sha1.c by Brad Conte
* Modified:   Single-round decomposition for compositional verification
*
* Key insight: Factor out the loop body as a single function.
* This allows verifying one round symbolically, then composing.
*********************************************************************/

#include <stdlib.h>
#include <memory.h>
#include "sha1.h"

/****************************** MACROS ******************************/
#define ROTLEFT(a, b) ((a << b) | (a >> (32 - b)))

/************************* DECOMPOSED TYPES *************************/
typedef struct {
    WORD a, b, c, d, e;
} SHA1_STATE;

/********************** SINGLE ROUND FUNCTIONS **********************/

// Ch function: (b & c) ^ (~b & d)
__attribute__((noinline))
WORD sha1_ch(WORD b, WORD c, WORD d) {
    return (b & c) ^ (~b & d);
}

// Parity function: b ^ c ^ d
__attribute__((noinline))
WORD sha1_parity(WORD b, WORD c, WORD d) {
    return b ^ c ^ d;
}

// Maj function: (b & c) ^ (b & d) ^ (c & d)
__attribute__((noinline))
WORD sha1_maj(WORD b, WORD c, WORD d) {
    return (b & c) ^ (b & d) ^ (c & d);
}

// Single round with Ch (rounds 0-19)
// Takes current state + one message word + constant, returns new state
__attribute__((noinline))
SHA1_STATE sha1_round_ch(SHA1_STATE s, WORD w, WORD k) {
    SHA1_STATE r;
    WORD t = ROTLEFT(s.a, 5) + sha1_ch(s.b, s.c, s.d) + s.e + k + w;
    r.a = t;
    r.b = s.a;
    r.c = ROTLEFT(s.b, 30);
    r.d = s.c;
    r.e = s.d;
    return r;
}

// Single round with Parity (rounds 20-39 and 60-79)
__attribute__((noinline))
SHA1_STATE sha1_round_parity(SHA1_STATE s, WORD w, WORD k) {
    SHA1_STATE r;
    WORD t = ROTLEFT(s.a, 5) + sha1_parity(s.b, s.c, s.d) + s.e + k + w;
    r.a = t;
    r.b = s.a;
    r.c = ROTLEFT(s.b, 30);
    r.d = s.c;
    r.e = s.d;
    return r;
}

// Single round with Maj (rounds 40-59)
__attribute__((noinline))
SHA1_STATE sha1_round_maj(SHA1_STATE s, WORD w, WORD k) {
    SHA1_STATE r;
    WORD t = ROTLEFT(s.a, 5) + sha1_maj(s.b, s.c, s.d) + s.e + k + w;
    r.a = t;
    r.b = s.a;
    r.c = ROTLEFT(s.b, 30);
    r.d = s.c;
    r.e = s.d;
    return r;
}

/********************** MESSAGE SCHEDULE ****************************/

__attribute__((noinline))
void sha1_message_schedule(const BYTE data[], WORD m[80]) {
    WORD i, j;
    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) + (data[j + 1] << 16) + (data[j + 2] << 8) + (data[j + 3]);
    for ( ; i < 80; ++i) {
        m[i] = (m[i - 3] ^ m[i - 8] ^ m[i - 14] ^ m[i - 16]);
        m[i] = (m[i] << 1) | (m[i] >> 31);
    }
}

/*********************** MAIN TRANSFORM *****************************/

__attribute__((noinline))
void sha1_transform(SHA1_CTX *ctx, const BYTE data[]) {
    WORD m[80];
    SHA1_STATE s;
    int i;

    sha1_message_schedule(data, m);

    s.a = ctx->state[0];
    s.b = ctx->state[1];
    s.c = ctx->state[2];
    s.d = ctx->state[3];
    s.e = ctx->state[4];

    // Rounds 0-19: Ch
    for (i = 0; i < 20; ++i)
        s = sha1_round_ch(s, m[i], ctx->k[0]);

    // Rounds 20-39: Parity
    for (i = 20; i < 40; ++i)
        s = sha1_round_parity(s, m[i], ctx->k[1]);

    // Rounds 40-59: Maj
    for (i = 40; i < 60; ++i)
        s = sha1_round_maj(s, m[i], ctx->k[2]);

    // Rounds 60-79: Parity
    for (i = 60; i < 80; ++i)
        s = sha1_round_parity(s, m[i], ctx->k[3]);

    ctx->state[0] += s.a;
    ctx->state[1] += s.b;
    ctx->state[2] += s.c;
    ctx->state[3] += s.d;
    ctx->state[4] += s.e;
}

/****************** UNCHANGED FROM ORIGINAL *************************/

void sha1_init(SHA1_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xc3d2e1f0;
    ctx->k[0] = 0x5a827999;
    ctx->k[1] = 0x6ed9eba1;
    ctx->k[2] = 0x8f1bbcdc;
    ctx->k[3] = 0xca62c1d6;
}

void sha1_update(SHA1_CTX *ctx, const BYTE data[], size_t len) {
    size_t i;
    for (i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha1_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

void sha1_final(SHA1_CTX *ctx, BYTE hash[]) {
    WORD i = ctx->datalen;

    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha1_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha1_transform(ctx, ctx->data);

    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
    }
}
