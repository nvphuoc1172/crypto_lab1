#include "dh_sample.h"

// ============================================================================
// Modular Exponentiation: (base^exp) % mod
// Using Binary Square-and-Multiply
// ============================================================================

uint512_t modular_exponentiation(const uint512_t &base, const uint512_t &exp, const uint512_t &mod)
{
    if (mod.is_zero() || mod.is_one())
    {
        return uint512_t(0);
    }

    size_t exp_len = exp.bit_length();
    if (exp_len == 0)
    {
        return uint512_t(1);
    }

    if (mod.is_odd())
    {
        uint512_montgomery_t montgomery(mod);
        if (montgomery.is_valid())
        {
            uint512_t result = montgomery.to_montgomery(uint512_t(1));
            const uint512_t cur = montgomery.to_montgomery(base);
            for (int i = (int)exp_len - 1; i >= 0; --i)
            {
                result = montgomery.multiply_montgomery(result, result);
                if (exp.bit((size_t)i))
                {
                    result = montgomery.multiply_montgomery(result, cur);
                }
            }
            return montgomery.from_montgomery(result);
        }
    }

    uint512_t result(1);
    uint512_t cur = uint1024_t::mod(uint1024_t(base), mod);

    for (int i = (int)exp_len - 1; i >= 0; i--)
    {
        result = uint1024_t::mul_mod(result, result, mod);
        if (exp.bit(i))
        {
            result = uint1024_t::mul_mod(result, cur, mod);
        }
    }

    return result;
}

// ============================================================================
// PRIVATE KEY GENERATION FOR DIFFIE-HELLMAN
// ============================================================================

namespace
{

uint512_t random_bits(prng_ctx *ctx, size_t bits)
{
    if (bits == 0)
    {
        return uint512_t(0);
    }
    if (bits > 512)
    {
        bits = 512;
    }

    uint512_t value;
    // Squeeze 8 consecutive 64-bit limbs (512 bits total) from SHAKE128
    for (size_t i = 0; i < 8; ++i)
    {
        value.digits[i] = prng_next_u64(ctx);
    }

    const size_t full_words = bits / 64;
    const size_t remaining_bits = bits % 64;

    // Mask out unused higher bits in the partially used word
    if (remaining_bits != 0)
    {
        value.digits[full_words] &= (uint64_t(1) << remaining_bits) - 1;
    }

    // Zero out all unused higher words
    const size_t first_unused_word = full_words + (remaining_bits != 0 ? 1 : 0);
    for (size_t i = first_unused_word; i < 8; ++i)
    {
        value.digits[i] = 0;
    }

    return value;
}

uint512_t random_below(prng_ctx *ctx, const uint512_t &limit)
{
    if (limit.is_zero())
    {
        return uint512_t(0);
    }

    const size_t bits = limit.bit_length();
    uint512_t value;
    do
    {
        value = random_bits(ctx, bits);
    } while (value >= limit);

    return value;
}
} // namespace

uint512_t generate_private_key(prng_ctx *ctx, const uint512_t &p)
{
    // Validate input parameters
    if (ctx == NULL || p < uint512_t(4))
    {
        return uint512_t(0);
    }

    // Number of elements in range [2, p - 2] is p - 3
    const uint512_t limit = p - uint512_t(3);

    // Uniform rejection sampling for r in [0, p - 4]
    const uint512_t r = random_below(ctx, limit);

    // Shift to interval [2, p - 2]
    return r + uint512_t(2);
}

uint512_t generate_private_key(const uint512_t &p)
{
    if (p < uint512_t(4))
    {
        return uint512_t(0);
    }

    prng_ctx ctx;
    uint8_t seed[32];
    randombytes(seed, sizeof(seed));
    prng_seed(&ctx, seed, sizeof(seed));

    return generate_private_key(&ctx, p);
}

