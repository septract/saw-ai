# SAW Verification Challenge Candidates

Research into historical and obscure ciphers suitable for SAW formal verification.

## Selection Criteria

- **Not in standard suites**: Avoid AES, SHA, etc. that have been heavily studied
- **Public C implementation**: Reference code must be available
- **Not formally verified**: No existing Cryptol specs or SAW verification work
- **Similar complexity to AES**: Avoid post-quantum or elliptic curve (complex arithmetic)
- **Interesting properties**: Ideally broken or with known weaknesses

---

## Tier 1: Top Candidates

### 1. FEAL (Fast Data Encipherment Algorithm) - ✅ COMPLETED

> **Status**: Fully verified! See [feal8-1989-verification.md](feal8-1989-verification.md)

**Background**: Designed by NTT Japan in 1987 as a faster alternative to DES. FEAL is historically significant as the cipher that led to the discovery of differential cryptanalysis by Biham and Shamir.

**Technical Details**:
- 64-bit block size, 64-bit key
- Feistel network structure (like DES)
- Originally 4 rounds (FEAL-4), extended to 8 (FEAL-8), then N rounds (FEAL-N/NX)
- Simple round function using addition, rotation, and XOR

**Vulnerabilities**:
- FEAL-4: Broken with differential cryptanalysis (Biham/Shamir, 1989)
- FEAL-8: Broken with 10,000 chosen plaintext pairs (Gilbert/Chassé, 1990)
- FEAL-N: Broken for N ≤ 31 (Biham/Shamir, 1991)
- Linear cryptanalysis first demonstrated on FEAL (Matsui/Yamagishi, 1992)

**C Implementations**:
- [theamazingking.com/crypto-feal.php](http://www.theamazingking.com/crypto-feal.php) - feal4full.c with differential cryptanalysis demo
- [Schneier's Applied Cryptography source](https://www.schneier.com/books/applied-cryptography-source/) - FEAL8.ZIP and FEALNX.ZIP
- [cocomelonc blog](https://cocomelonc.github.io/malware/2024/09/12/malware-cryptography-32.html) - FEAL-8 C implementation

**Formal Verification Status**: ✅ **COMPLETED** - Full SAW verification of 1989 implementation against HAC reference spec. See [experiments/feal/](../experiments/feal/).

**Why it's ideal**:
- Historic importance makes it educational
- Simple enough to verify completely
- Well-documented attacks enable interesting verification goals
- Could verify implementation correctness AND model attack characteristics
- Feistel structure similar to DES, provides good SAW experience

---

### 2. RC5 (Rivest Cipher 5)

**Background**: Designed by Ron Rivest in 1994, notable for its simplicity and use of data-dependent rotations. Was the basis for RC6 (AES finalist).

**Technical Details**:
- Variable block size (32, 64, or 128 bits)
- Variable key size (0 to 2040 bits)
- Variable rounds (0 to 255, recommended 12-20)
- Key feature: data-dependent rotations
- Uses only: addition, XOR, rotation

**Vulnerabilities**:
- 12-round version vulnerable to differential attack with 2^44 chosen plaintexts
- Weak keys exist with respect to differential attacks
- Timing attacks possible due to data-dependent rotations
- RC5-56 brute forced in 250 days (1997)
- RC5-64 brute forced in 1757 days (2002)

**C Implementation**:
- Reference implementation in RFC 2040 appendix
- Original C code from Rivest's paper

**Formal Verification Status**: None found.

**Why it's interesting**:
- Data-dependent rotations are unusual and interesting for SMT solvers
- Simple structure with only 3 operations
- Well-documented in RFC with reference C code
- Known vulnerabilities provide verification targets

---

### 3. Nimbus

**Background**: Designed by Alexis Machado in 2000, submitted to NESSIE competition. Completely broken shortly after submission.

**Technical Details**:
- 64-bit block size, 128-bit key
- Only 5 rounds
- Round function: XOR with subkey, reverse bit order, multiply mod 2^64

**Vulnerabilities**:
- **Completely broken**: Differential attack with only 256 chosen plaintexts!
- Attack complexity: 2^10 (about 1024 operations)
- Attack by Biham and Furman discovered fundamental weakness in multiplication as mixing operation

**C Implementation**:
- PicoCTF 2021 "Clouds" challenge uses Nimbus (Python, could be ported)
- Original NESSIE submission may contain reference code

**Formal Verification Status**: None found.

**Why it's interesting**:
- Extremely simple structure
- Catastrophically broken - could verify the attack works!
- Educational example of poor cipher design
- Could complete verification very quickly

---

## Tier 2: Good Alternatives

### 4. Blowfish

**Background**: Designed by Bruce Schneier in 1993. Widely deployed, now considered legacy.

**Technical Details**:
- 64-bit block, variable key (32-448 bits)
- 16-round Feistel network
- Key-dependent S-boxes (requires 521 encryptions to set up)

**Vulnerabilities**:
- SWEET32 birthday attack (2016) due to 64-bit block size
- CVE-2016-2183 (TLS), CVE-2016-6329 (OpenVPN)
- No attack on cipher itself, just block size issue

**C Implementation**: Already in B-Con crypto-algorithms repo!

**Formal Verification Status**: None found with SAW/Cryptol.

**Consideration**: Available in B-Con but S-box setup is complex.

---

### 5. NUSH

**Background**: Russian cipher by LAN Crypto, submitted to NESSIE. Rejected due to linear cryptanalysis vulnerability.

**Technical Details**:
- Variable block (64/128/256 bits), variable key (128/192/256 bits)
- No S-boxes - only AND, OR, XOR, addition, rotation
- 9, 17, or 33 rounds depending on block size

**Vulnerabilities**:
- Broken by linear cryptanalysis (faster than brute force)

**C Implementation**: Would need to locate NESSIE submission archive.

**Formal Verification Status**: None found.

**Consideration**: Interesting due to lack of S-boxes.

---

### 6. KASUMI

**Background**: Modified MISTY1, used in 3G/GSM mobile networks (A5/3 algorithm).

**Technical Details**:
- 64-bit block, 128-bit key
- 8-round Feistel network

**Vulnerabilities**:
- Related-key attack breaks full cipher in under 2 hours on single PC (Dunkelman/Keller/Shamir, 2010)
- Original MISTY1 would have been immune to this attack
- Modifications for KASUMI introduced the weakness

**C Implementation**:
- Official 3GPP/ETSI specification includes test vectors
- NXP application note AN2837 has C implementation

**Formal Verification Status**: None found.

**Consideration**: Real-world deployed cipher with known weakness.

---

### 7. Anubis

**Background**: By Rijmen and Barreto (Rijndael designers), NESSIE submission. Not selected due to similarity to Rijndael rather than weakness.

**Technical Details**:
- 128-bit block, variable key (128-320 bits)
- Substitution-permutation network (very similar to AES)

**Vulnerabilities**: None known - rejected for being "too similar to AES"

**C Implementation**: In Linux kernel! (`linux/crypto/anubis.c`)

**Formal Verification Status**: None found.

**Consideration**: Would be good "AES alternative" verification, public domain.

---

## Tier 3: Other Interesting Options

| Cipher | Status | Notes |
|--------|--------|-------|
| Hierocrypt | NESSIE reject | Key schedule weakness, square attacks on reduced rounds |
| Grand Cru | NESSIE reject | Rijndael variant, not broken but not selected |
| Q | NESSIE reject | Differential attack with 2^77 complexity |
| SAFER | Pre-AES | Various versions, some weak key schedules |
| MISTY1 | CRYPTREC | Broken in 2015 by integral cryptanalysis |

---

## Recommendation

**Primary recommendation: FEAL**

FEAL is the ideal choice because:

1. **Historic significance**: The cipher where differential cryptanalysis was invented. This provides educational value and context.

2. **Well-documented attacks**: Multiple published attacks with varying complexity, providing interesting verification targets beyond just "implementation matches spec."

3. **Appropriate complexity**: Feistel structure similar to DES, round function is simple but not trivial. 64-bit blocks keep state space manageable.

4. **Multiple C implementations available**: Reference code from Applied Cryptography, tutorial implementations, etc.

5. **No existing formal verification**: Fresh target for SAW work.

6. **Verification possibilities**:
   - Verify C implementation matches specification
   - Potentially model and verify differential characteristics
   - Could verify attack implementation
   - Educational demonstration of "what formal verification of broken cipher looks like"

**Secondary recommendation: RC5**

If FEAL proves problematic (e.g., hard to find clean C code), RC5 is excellent because:
- RFC 2040 has official reference C implementation
- Data-dependent rotations are interesting for SAT/SMT
- Well-documented vulnerability profile

**Fallback: Nimbus**

If you want something that can be done quickly and demonstrates verification of a completely broken cipher, Nimbus is ideal. The attack is so efficient (256 chosen plaintexts) that you could potentially verify the attack itself.

---

## Next Steps

1. Obtain clean C implementation of FEAL-4 or FEAL-8
2. Write Cryptol specification from FEAL paper/documentation
3. Start with concrete tests to validate spec matches implementation
4. Move to symbolic verification of primitives
5. Consider verifying differential characteristics as advanced goal

## Sources

- [NESSIE Project](https://www.cosic.esat.kuleuven.be/nessie/)
- [Schneier's Applied Cryptography Source Code](https://www.schneier.com/books/applied-cryptography-source/)
- [Differential Cryptanalysis of FEAL](http://www.theamazingking.com/crypto-feal.php)
- [RFC 2040 - RC5 Specification](https://datatracker.ietf.org/doc/html/rfc2040)
- [Differential Cryptanalysis of Nimbus (Biham/Furman)](https://www.researchgate.net/publication/220942467_Differential_Cryptanalysis_of_Nimbus)
- [Wikipedia: NESSIE](https://en.wikipedia.org/wiki/NESSIE)
- [Sweet32 Attack](https://sweet32.info/)
