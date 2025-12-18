/*
 * AES PBT Test Harness
 *
 * Provides SCALAR wrapper functions for interior AES functions.
 * These wrappers use only scalar types (uint64_t, uint32_t, uint8_t)
 * so that llvm_extract can extract them as pure Cryptol functions.
 *
 * The wrappers handle memory setup/teardown internally.
 * SAW only sees scalars in and scalars out.
 */

#include "../repo/aes.c"
#include <stdint.h>
#include <string.h>

/*
 * State packing convention:
 * AES state is 4x4 bytes = 16 bytes = 128 bits
 * We pack as two 64-bit values: state_lo (bytes 0-7), state_hi (bytes 8-15)
 *
 * The C code uses state[row][col] where state is BYTE[4][4]
 * Bytes are laid out in memory as: state[0][0..3], state[1][0..3], ...
 *
 * Pack order (big-endian within each uint64):
 *   state_lo = state[0][0..3] || state[1][0..3]  (8 bytes)
 *   state_hi = state[2][0..3] || state[3][0..3]  (8 bytes)
 */

// Helper: unpack two uint64 into state[4][4]
static void unpack_state(uint64_t lo, uint64_t hi, BYTE state[4][4]) {
    state[0][0] = (lo >> 56) & 0xFF;
    state[0][1] = (lo >> 48) & 0xFF;
    state[0][2] = (lo >> 40) & 0xFF;
    state[0][3] = (lo >> 32) & 0xFF;
    state[1][0] = (lo >> 24) & 0xFF;
    state[1][1] = (lo >> 16) & 0xFF;
    state[1][2] = (lo >> 8) & 0xFF;
    state[1][3] = lo & 0xFF;

    state[2][0] = (hi >> 56) & 0xFF;
    state[2][1] = (hi >> 48) & 0xFF;
    state[2][2] = (hi >> 40) & 0xFF;
    state[2][3] = (hi >> 32) & 0xFF;
    state[3][0] = (hi >> 24) & 0xFF;
    state[3][1] = (hi >> 16) & 0xFF;
    state[3][2] = (hi >> 8) & 0xFF;
    state[3][3] = hi & 0xFF;
}

// Helper: pack state[4][4] into two uint64
static uint64_t pack_state_lo(BYTE state[4][4]) {
    return ((uint64_t)state[0][0] << 56) |
           ((uint64_t)state[0][1] << 48) |
           ((uint64_t)state[0][2] << 40) |
           ((uint64_t)state[0][3] << 32) |
           ((uint64_t)state[1][0] << 24) |
           ((uint64_t)state[1][1] << 16) |
           ((uint64_t)state[1][2] << 8) |
           ((uint64_t)state[1][3]);
}

static uint64_t pack_state_hi(BYTE state[4][4]) {
    return ((uint64_t)state[2][0] << 56) |
           ((uint64_t)state[2][1] << 48) |
           ((uint64_t)state[2][2] << 40) |
           ((uint64_t)state[2][3] << 32) |
           ((uint64_t)state[3][0] << 24) |
           ((uint64_t)state[3][1] << 16) |
           ((uint64_t)state[3][2] << 8) |
           ((uint64_t)state[3][3]);
}

/*
 * =============================================================================
 * SBox - single byte substitution (simplest case)
 * =============================================================================
 */
uint8_t pbt_SBox(uint8_t in) {
    return aes_sbox[in >> 4][in & 0x0F];
}

uint8_t pbt_InvSBox(uint8_t in) {
    return aes_invsbox[in >> 4][in & 0x0F];
}

/*
 * =============================================================================
 * SubBytes - operates on full state
 * =============================================================================
 */
uint64_t pbt_SubBytes_lo(uint64_t in_lo, uint64_t in_hi) {
    BYTE state[4][4];
    unpack_state(in_lo, in_hi, state);
    SubBytes(state);
    return pack_state_lo(state);
}

uint64_t pbt_SubBytes_hi(uint64_t in_lo, uint64_t in_hi) {
    BYTE state[4][4];
    unpack_state(in_lo, in_hi, state);
    SubBytes(state);
    return pack_state_hi(state);
}

uint64_t pbt_InvSubBytes_lo(uint64_t in_lo, uint64_t in_hi) {
    BYTE state[4][4];
    unpack_state(in_lo, in_hi, state);
    InvSubBytes(state);
    return pack_state_lo(state);
}

uint64_t pbt_InvSubBytes_hi(uint64_t in_lo, uint64_t in_hi) {
    BYTE state[4][4];
    unpack_state(in_lo, in_hi, state);
    InvSubBytes(state);
    return pack_state_hi(state);
}

/*
 * =============================================================================
 * ShiftRows - operates on full state
 * =============================================================================
 */
uint64_t pbt_ShiftRows_lo(uint64_t in_lo, uint64_t in_hi) {
    BYTE state[4][4];
    unpack_state(in_lo, in_hi, state);
    ShiftRows(state);
    return pack_state_lo(state);
}

uint64_t pbt_ShiftRows_hi(uint64_t in_lo, uint64_t in_hi) {
    BYTE state[4][4];
    unpack_state(in_lo, in_hi, state);
    ShiftRows(state);
    return pack_state_hi(state);
}

uint64_t pbt_InvShiftRows_lo(uint64_t in_lo, uint64_t in_hi) {
    BYTE state[4][4];
    unpack_state(in_lo, in_hi, state);
    InvShiftRows(state);
    return pack_state_lo(state);
}

uint64_t pbt_InvShiftRows_hi(uint64_t in_lo, uint64_t in_hi) {
    BYTE state[4][4];
    unpack_state(in_lo, in_hi, state);
    InvShiftRows(state);
    return pack_state_hi(state);
}

/*
 * =============================================================================
 * MixColumns - operates on full state
 * =============================================================================
 */
uint64_t pbt_MixColumns_lo(uint64_t in_lo, uint64_t in_hi) {
    BYTE state[4][4];
    unpack_state(in_lo, in_hi, state);
    MixColumns(state);
    return pack_state_lo(state);
}

uint64_t pbt_MixColumns_hi(uint64_t in_lo, uint64_t in_hi) {
    BYTE state[4][4];
    unpack_state(in_lo, in_hi, state);
    MixColumns(state);
    return pack_state_hi(state);
}

uint64_t pbt_InvMixColumns_lo(uint64_t in_lo, uint64_t in_hi) {
    BYTE state[4][4];
    unpack_state(in_lo, in_hi, state);
    InvMixColumns(state);
    return pack_state_lo(state);
}

uint64_t pbt_InvMixColumns_hi(uint64_t in_lo, uint64_t in_hi) {
    BYTE state[4][4];
    unpack_state(in_lo, in_hi, state);
    InvMixColumns(state);
    return pack_state_hi(state);
}

/*
 * =============================================================================
 * AddRoundKey - takes state and 4 key words (128 bits each)
 * Key is packed as w0, w1, w2, w3 (each 32 bits)
 * We pack key as two 64-bit values: key_lo = w0||w1, key_hi = w2||w3
 * =============================================================================
 */
uint64_t pbt_AddRoundKey_lo(uint64_t state_lo, uint64_t state_hi,
                            uint64_t key_lo, uint64_t key_hi) {
    BYTE state[4][4];
    WORD w[4];

    unpack_state(state_lo, state_hi, state);

    // Unpack key words (big-endian)
    w[0] = (key_lo >> 32) & 0xFFFFFFFF;
    w[1] = key_lo & 0xFFFFFFFF;
    w[2] = (key_hi >> 32) & 0xFFFFFFFF;
    w[3] = key_hi & 0xFFFFFFFF;

    AddRoundKey(state, w);
    return pack_state_lo(state);
}

uint64_t pbt_AddRoundKey_hi(uint64_t state_lo, uint64_t state_hi,
                            uint64_t key_lo, uint64_t key_hi) {
    BYTE state[4][4];
    WORD w[4];

    unpack_state(state_lo, state_hi, state);

    w[0] = (key_lo >> 32) & 0xFFFFFFFF;
    w[1] = key_lo & 0xFFFFFFFF;
    w[2] = (key_hi >> 32) & 0xFFFFFFFF;
    w[3] = key_hi & 0xFFFFFFFF;

    AddRoundKey(state, w);
    return pack_state_hi(state);
}

/*
 * =============================================================================
 * SubWord - already scalar (WORD -> WORD), but wrap for consistency
 * =============================================================================
 */
uint32_t pbt_SubWord(uint32_t word) {
    return SubWord(word);
}

/*
 * =============================================================================
 * Key expansion - single round step
 * This tests the core key expansion logic for one iteration
 *
 * For round i where i % Nk == 0:
 *   temp = SubWord(RotWord(w[i-1])) ^ Rcon[i/Nk]
 *   w[i] = w[i-Nk] ^ temp
 *
 * We expose this as: given w_prev (w[i-1]) and w_nk (w[i-Nk]) and round_constant,
 * compute w_i
 * =============================================================================
 */
uint32_t pbt_KeyExpansionStep(uint32_t w_prev, uint32_t w_nk, uint32_t rcon) {
    // RotWord: rotate left by 8 bits
    uint32_t temp = ((w_prev << 8) | (w_prev >> 24));
    // SubWord
    temp = SubWord(temp);
    // XOR with Rcon
    temp ^= rcon;
    // XOR with w[i-Nk]
    return w_nk ^ temp;
}

/*
 * For rounds where i % Nk != 0 (and not the special case for Nk > 6):
 *   w[i] = w[i-1] ^ w[i-Nk]
 */
uint32_t pbt_KeyExpansionStepSimple(uint32_t w_prev, uint32_t w_nk) {
    return w_prev ^ w_nk;
}

/*
 * =============================================================================
 * Full Key Expansion (AES-128)
 * Returns specific words from the expanded key schedule
 * Key schedule has 44 words (w[0] through w[43])
 * =============================================================================
 */

// Helper: unpack 128-bit key from two uint64 into 16 bytes
static void unpack_key(uint64_t lo, uint64_t hi, BYTE key[16]) {
    key[0]  = (lo >> 56) & 0xFF;
    key[1]  = (lo >> 48) & 0xFF;
    key[2]  = (lo >> 40) & 0xFF;
    key[3]  = (lo >> 32) & 0xFF;
    key[4]  = (lo >> 24) & 0xFF;
    key[5]  = (lo >> 16) & 0xFF;
    key[6]  = (lo >> 8) & 0xFF;
    key[7]  = lo & 0xFF;
    key[8]  = (hi >> 56) & 0xFF;
    key[9]  = (hi >> 48) & 0xFF;
    key[10] = (hi >> 40) & 0xFF;
    key[11] = (hi >> 32) & 0xFF;
    key[12] = (hi >> 24) & 0xFF;
    key[13] = (hi >> 16) & 0xFF;
    key[14] = (hi >> 8) & 0xFF;
    key[15] = hi & 0xFF;
}

// Helper: unpack 128-bit block from two uint64 into 16 bytes
static void unpack_block(uint64_t lo, uint64_t hi, BYTE block[16]) {
    unpack_key(lo, hi, block);  // Same format
}

// Helper: pack 16 bytes into two uint64
static uint64_t pack_block_lo(BYTE block[16]) {
    return ((uint64_t)block[0] << 56) |
           ((uint64_t)block[1] << 48) |
           ((uint64_t)block[2] << 40) |
           ((uint64_t)block[3] << 32) |
           ((uint64_t)block[4] << 24) |
           ((uint64_t)block[5] << 16) |
           ((uint64_t)block[6] << 8) |
           ((uint64_t)block[7]);
}

static uint64_t pack_block_hi(BYTE block[16]) {
    return ((uint64_t)block[8] << 56) |
           ((uint64_t)block[9] << 48) |
           ((uint64_t)block[10] << 40) |
           ((uint64_t)block[11] << 32) |
           ((uint64_t)block[12] << 24) |
           ((uint64_t)block[13] << 16) |
           ((uint64_t)block[14] << 8) |
           ((uint64_t)block[15]);
}

// Get a specific word from the key schedule
// For AES-128, word_index ranges from 0 to 43
uint32_t pbt_KeyScheduleWord(uint64_t key_lo, uint64_t key_hi, uint32_t word_index) {
    BYTE key[16];
    WORD key_schedule[60];  // Max size for AES-256

    unpack_key(key_lo, key_hi, key);
    aes_key_setup(key, key_schedule, 128);

    // Clamp to valid range
    if (word_index > 43) word_index = 43;

    return key_schedule[word_index];
}

// Get a full round key (4 words) as two uint64 values
// For AES-128, round ranges from 0 to 10
uint64_t pbt_RoundKey_lo(uint64_t key_lo, uint64_t key_hi, uint32_t round) {
    BYTE key[16];
    WORD key_schedule[60];

    unpack_key(key_lo, key_hi, key);
    aes_key_setup(key, key_schedule, 128);

    if (round > 10) round = 10;

    // Round key starts at w[round * 4]
    uint32_t base = round * 4;
    return ((uint64_t)key_schedule[base] << 32) | key_schedule[base + 1];
}

uint64_t pbt_RoundKey_hi(uint64_t key_lo, uint64_t key_hi, uint32_t round) {
    BYTE key[16];
    WORD key_schedule[60];

    unpack_key(key_lo, key_hi, key);
    aes_key_setup(key, key_schedule, 128);

    if (round > 10) round = 10;

    uint32_t base = round * 4;
    return ((uint64_t)key_schedule[base + 2] << 32) | key_schedule[base + 3];
}

/*
 * =============================================================================
 * Full AES-128 Encryption/Decryption
 * =============================================================================
 */
uint64_t pbt_aes_encrypt_lo(uint64_t pt_lo, uint64_t pt_hi,
                            uint64_t key_lo, uint64_t key_hi) {
    BYTE plaintext[16], ciphertext[16], key[16];
    WORD key_schedule[60];

    unpack_block(pt_lo, pt_hi, plaintext);
    unpack_key(key_lo, key_hi, key);

    aes_key_setup(key, key_schedule, 128);
    aes_encrypt(plaintext, ciphertext, key_schedule, 128);

    return pack_block_lo(ciphertext);
}

uint64_t pbt_aes_encrypt_hi(uint64_t pt_lo, uint64_t pt_hi,
                            uint64_t key_lo, uint64_t key_hi) {
    BYTE plaintext[16], ciphertext[16], key[16];
    WORD key_schedule[60];

    unpack_block(pt_lo, pt_hi, plaintext);
    unpack_key(key_lo, key_hi, key);

    aes_key_setup(key, key_schedule, 128);
    aes_encrypt(plaintext, ciphertext, key_schedule, 128);

    return pack_block_hi(ciphertext);
}

uint64_t pbt_aes_decrypt_lo(uint64_t ct_lo, uint64_t ct_hi,
                            uint64_t key_lo, uint64_t key_hi) {
    BYTE ciphertext[16], plaintext[16], key[16];
    WORD key_schedule[60];

    unpack_block(ct_lo, ct_hi, ciphertext);
    unpack_key(key_lo, key_hi, key);

    aes_key_setup(key, key_schedule, 128);
    aes_decrypt(ciphertext, plaintext, key_schedule, 128);

    return pack_block_lo(plaintext);
}

uint64_t pbt_aes_decrypt_hi(uint64_t ct_lo, uint64_t ct_hi,
                            uint64_t key_lo, uint64_t key_hi) {
    BYTE ciphertext[16], plaintext[16], key[16];
    WORD key_schedule[60];

    unpack_block(ct_lo, ct_hi, ciphertext);
    unpack_key(key_lo, key_hi, key);

    aes_key_setup(key, key_schedule, 128);
    aes_decrypt(ciphertext, plaintext, key_schedule, 128);

    return pack_block_hi(plaintext);
}
