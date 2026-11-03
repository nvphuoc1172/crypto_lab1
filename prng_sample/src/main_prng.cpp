#include "prng.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

int main()
{
    uint8_t seed[32];

    // you can use a fixed seed for this lab and for learning purposes.
    memcpy(seed, "seed", sizeof("seed"));

    prng_ctx ctx;
    prng_seed(&ctx, seed, sizeof(seed));

    uint16_t r1 = prng_next_u16(&ctx);
    uint32_t r2 = prng_next_u32(&ctx);
    uint64_t r3 = prng_next_u64(&ctx);

    printf("Random 16-bit number: %u\n", r1);
    printf("Random 32-bit number: %u\n", r2);
    printf("Random 64-bit number: %" PRIu64 "\n", r3);
    return 0;
}
