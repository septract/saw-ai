/*
 * FEAL-8 Implementation (September 1989)
 *
 * Source: https://www.schneier.com/wp-content/uploads/2015/03/FEAL8-2.zip
 * From: Schneier's Applied Cryptography source code archive
 * Author: Unknown
 *
 * Code characteristics:
 * - Lookup table for Rot2 (lazy-initialized 256-entry table)
 * - Union-based byte manipulation (assumes sizeof(long)==4, endian-dependent)
 * - Global state for expanded key
 *
 * PORTABILITY NOTE (2024):
 * The original code assumed ILP32 (sizeof(long)==4). On LP64 systems (modern
 * 64-bit Unix), sizeof(long)==8, causing unions like { unsigned long All;
 * ByteType Byte[4]; } to have uninitialized high bytes when Byte[] is written
 * and All is read. We've added zero-initialization to affected unions to make
 * the code portable to 64-bit systems. See comments at MakeH1, f, FK, SetKey.
 */

#include <stdio.h>

typedef unsigned char ByteType;
typedef unsigned long HalfWord;
typedef unsigned int QuarterWord;

/* Global key state */
QuarterWord K[16];
HalfWord K89, K1011, K1213, K1415;

/* Forward declarations */
void SetKey(ByteType *);
void Encrypt(ByteType *Plain, ByteType *Cipher);
void Decrypt(ByteType *Cipher, ByteType *Plain);
HalfWord MakeH1(ByteType *);
HalfWord MakeH2(QuarterWord *);
HalfWord f(HalfWord, QuarterWord);
HalfWord FK(HalfWord, HalfWord);
void DissH1(HalfWord, ByteType *);
void DissQ1(QuarterWord, ByteType *);
ByteType Rot2(ByteType);
ByteType S0(ByteType, ByteType);
ByteType S1(ByteType, ByteType);

/*
 * Rot2 - 2-bit left rotation using LOOKUP TABLE
 * This is the key structural difference from the HAC implementation!
 */
ByteType Rot2(ByteType X)
{
    static int First = 1;
    static ByteType RetVal[256];

    if (First)
    {
        int i, High, Low;
        for (i = 0, High = 0, Low = 0; i < 256; ++i)
        {
            RetVal[i] = High + Low;
            High += 4;
            if (High > 255)
            {
                High = 0;
                ++Low;
            }
        }
        First = 0;
    }
    return RetVal[X];
}

ByteType S0(ByteType X1, ByteType X2)
{
    return Rot2((X1 + X2) & 0xff);
}

ByteType S1(ByteType X1, ByteType X2)
{
    return Rot2((X1 + X2 + 1) & 0xff);
}

HalfWord MakeH1(ByteType *B)
/*
     Assemble a HalfWord from the four bytes provided.
*/
{
    /*
     * PORTABILITY FIX: On LP64 systems (64-bit), unsigned long is 8 bytes
     * but Byte[4] is only 4 bytes. Without initialization, reading All after
     * writing Byte[0-3] returns 4 uninitialized bytes in the high bits.
     * This code worked correctly on ILP32 (1989) where sizeof(long)==4.
     * Zero-initialization makes the code portable to 64-bit systems.
     */
    union {
        unsigned long All;
        ByteType Byte[4];
    } RetVal = {0};

    RetVal.Byte[0] = *B++;
    RetVal.Byte[1] = *B++;
    RetVal.Byte[2] = *B++;
    RetVal.Byte[3] = *B;
    return RetVal.All;
}

HalfWord MakeH2(QuarterWord *Q)
/*
     Make a halfword from the two quarterwords given.
*/
{
    ByteType B[4];

    DissQ1(*Q++, B);
    DissQ1(*Q, B + 2);
    return MakeH1(B);
}

void DissH1(HalfWord H, ByteType *D)
/*
     Disassemble the given halfword into 4 bytes.
*/
{
    union {
        HalfWord All;
        ByteType Byte[4];
    } T;

    T.All = H;
    *D++ = T.Byte[0];
    *D++ = T.Byte[1];
    *D++ = T.Byte[2];
    *D = T.Byte[3];
}

void DissQ1(QuarterWord Q, ByteType *B)
/*
     Disassemble a quarterword into two Bytes.
*/
{
    union {
        QuarterWord All;
        ByteType Byte[2];
    } QQ;

    QQ.All = Q;
    *B++ = QQ.Byte[0];
    *B = QQ.Byte[1];
}

HalfWord f(HalfWord AA, QuarterWord BB)
/*
     Evaluate the f function.
*/
{
    ByteType f1, f2;
    /*
     * PORTABILITY FIX: See MakeH1 comment. RetVal needs initialization
     * because we write Byte[0-3] then read All on 64-bit systems.
     * A doesn't need it because we write All first, then read Byte[].
     */
    union {
        unsigned long All;
        ByteType Byte[4];
    } RetVal = {0}, A;
    union {
        unsigned int All;
        ByteType Byte[2];
    } B;

    A.All = AA;
    B.All = BB;
    f1 = A.Byte[1] ^ B.Byte[0] ^ A.Byte[0];
    f2 = A.Byte[2] ^ B.Byte[1] ^ A.Byte[3];
    f1 = S1(f1, f2);
    f2 = S0(f2, f1);
    RetVal.Byte[1] = f1;
    RetVal.Byte[2] = f2;
    RetVal.Byte[0] = S0(A.Byte[0], f1);
    RetVal.Byte[3] = S1(A.Byte[3], f2);
    return RetVal.All;
}

HalfWord FK(HalfWord AA, HalfWord BB)
/*
     Evaluate the FK function.
*/
{
    ByteType FK1, FK2;
    /*
     * PORTABILITY FIX: See MakeH1 comment. RetVal needs initialization
     * because we write Byte[0-3] then read All on 64-bit systems.
     * A and B don't need it because we write All first, then read Byte[].
     */
    union {
        unsigned long All;
        ByteType Byte[4];
    } RetVal = {0}, A, B;

    A.All = AA;
    B.All = BB;
    FK1 = A.Byte[1] ^ A.Byte[0];
    FK2 = A.Byte[2] ^ A.Byte[3];
    FK1 = S1(FK1, FK2 ^ B.Byte[0]);
    FK2 = S0(FK2, FK1 ^ B.Byte[1]);
    RetVal.Byte[1] = FK1;
    RetVal.Byte[2] = FK2;
    RetVal.Byte[0] = S0(A.Byte[0], FK1 ^ B.Byte[2]);
    RetVal.Byte[3] = S1(A.Byte[3], FK2 ^ B.Byte[3]);
    return RetVal.All;
}

void SetKey(ByteType *KP)
/*
     KP points to an array of 8 bytes.
*/
{
    /*
     * PORTABILITY FIX: See MakeH1 comment. A and B need initialization
     * because we write Byte[0-3] then read All. Q needs initialization
     * because we write Byte[0-1] then read All (QuarterWord is 4 bytes on LP64).
     * D is explicitly set to 0. NewB gets its value from FK return.
     */
    union {
        HalfWord All;
        ByteType Byte[4];
    } A = {0}, B = {0}, D, NewB;
    union {
        QuarterWord All;
        ByteType Byte[2];
    } Q = {0};
    int i;
    QuarterWord *Out;

    A.Byte[0] = *KP++;
    A.Byte[1] = *KP++;
    A.Byte[2] = *KP++;
    A.Byte[3] = *KP++;
    B.Byte[0] = *KP++;
    B.Byte[1] = *KP++;
    B.Byte[2] = *KP++;
    B.Byte[3] = *KP;
    D.All = 0;

    for (i = 1, Out = K; i <= 8; ++i)
    {
        NewB.All = FK(A.All, B.All ^ D.All);
        D = A;
        A = B;
        B = NewB;
        Q.Byte[0] = B.Byte[0];
        Q.Byte[1] = B.Byte[1];
        *Out++ = Q.All;
        Q.Byte[0] = B.Byte[2];
        Q.Byte[1] = B.Byte[3];
        *Out++ = Q.All;
    }
    K89 = MakeH2(K + 8);
    K1011 = MakeH2(K + 10);
    K1213 = MakeH2(K + 12);
    K1415 = MakeH2(K + 14);
}

void Encrypt(ByteType *Plain, ByteType *Cipher)
/*
     Encrypt a block, using the last key set.
*/
{
    HalfWord L, R, NewR;
    int r;

    L = MakeH1(Plain);
    R = MakeH1(Plain + 4);
    L ^= K89;
    R ^= K1011;
    R ^= L;

    for (r = 0; r < 8; ++r)
    {
        NewR = L ^ f(R, K[r]);
        L = R;
        R = NewR;
    }

    L ^= R;
    R ^= K1213;
    L ^= K1415;

    DissH1(R, Cipher);
    DissH1(L, Cipher + 4);
}

void Decrypt(ByteType *Cipher, ByteType *Plain)
/*
     Decrypt a block, using the last key set.
*/
{
    HalfWord L, R, NewL;
    int r;

    R = MakeH1(Cipher);
    L = MakeH1(Cipher + 4);
    R ^= K1213;
    L ^= K1415;
    L ^= R;

    for (r = 7; r >= 0; --r)
    {
        NewL = R ^ f(L, K[r]);
        R = L;
        L = NewL;
    }

    R ^= L;
    R ^= K1011;
    L ^= K89;

    DissH1(L, Plain);
    DissH1(R, Plain + 4);
}

/* Test harness */
int main(void)
{
    ByteType key[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
    ByteType plain[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    ByteType cipher[8];
    ByteType decrypted[8];
    int i;

    printf("FEAL-8 (1989 Implementation) Test\n");
    printf("==================================\n\n");

    printf("Key:       ");
    for (i = 0; i < 8; i++) printf("%02x", key[i]);
    printf("\n");

    printf("Plaintext: ");
    for (i = 0; i < 8; i++) printf("%02x", plain[i]);
    printf("\n");

    SetKey(key);

    printf("\nSubkeys:   ");
    for (i = 0; i < 16; i++) printf("%04x ", K[i]);
    printf("\n");

    Encrypt(plain, cipher);

    printf("\nCiphertext: ");
    for (i = 0; i < 8; i++) printf("%02x", cipher[i]);
    printf("\n");

    Decrypt(cipher, decrypted);

    printf("Decrypted:  ");
    for (i = 0; i < 8; i++) printf("%02x", decrypted[i]);
    printf("\n");

    /* Verify round-trip */
    int match = 1;
    for (i = 0; i < 8; i++) {
        if (plain[i] != decrypted[i]) match = 0;
    }
    printf("\nRound-trip: %s\n", match ? "PASS" : "FAIL");

    return match ? 0 : 1;
}
