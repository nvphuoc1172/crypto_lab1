#include "prng.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main()
{
    // Zero-length requests must not access the output buffer.
    randombytes(NULL, 0);
    uint8_t guarded[34] = {};
    guarded[0] = 0x5a;
    guarded[33] = 0xa5;
    randombytes(guarded + 1, 32);
    assert(guarded[0] == 0x5a && guarded[33] == 0xa5);

    // Validate the PRNG wrappers against a single SHAKE128 byte stream.
    uint8_t seed[32] = {};
    memcpy(seed, "seed", 4);
    uint8_t expected[14];
    shake128(expected, sizeof(expected), seed, sizeof(seed));
    prng_ctx ctx;
    prng_seed(&ctx, seed, sizeof(seed));
    uint64_t values[] = {prng_next_u16(&ctx), prng_next_u32(&ctx), prng_next_u64(&ctx)};
    size_t offset = 0;
    const size_t sizes[] = {2, 4, 8};
    for (size_t n = 0; n < 3; n++)
    {
        uint64_t value = 0;
        for (size_t i = 0; i < sizes[n]; i++)
            value |= (uint64_t)expected[offset++] << (8 * i);
        assert(values[n] == value);
    }

    // An OS-generated seed can also initialize the SHAKE state.
    prng_seed(&ctx, guarded + 1, 32);
    (void)prng_next_u64(&ctx);
    puts("PRNG checks passed.");
    return 0;
}
