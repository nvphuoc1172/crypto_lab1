#include "dh_sample.h"

uint512_t generate_private_key(prng_ctx *ctx, const uint512_t &p)
{
    if (ctx == NULL || p < uint512_t(5))
    {
        return uint512_t(0);
    }

    const uint512_t range = p - uint512_t(3);
    const size_t bits = (range - uint512_t(1)).bit_length();
    uint512_t candidate;
    do
    {
        candidate = uint512_t(0);
        for (size_t i = 0; i < (bits + 63) / 64; ++i)
        {
            candidate.digits[i] = prng_next_u64(ctx);
        }
        if (bits % 64 != 0)
        {
            candidate.digits[bits / 64] &= (uint64_t(1) << (bits % 64)) - 1;
        }
        // Reject out-of-range values instead of reducing modulo range,
        // which would give some keys a higher probability than others.
    } while (candidate >= range);

    return candidate + uint512_t(2);
}
