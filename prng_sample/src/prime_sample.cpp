#include "dh_sample.h"

namespace
{
const uint16_t SMALL_PRIMES[] = {
    3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47,
    53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107,
    109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167,
    173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
    233, 239, 241, 251
};

uint512_t random_bits(prng_ctx *ctx, size_t bits)
{
    uint512_t value;
    for (size_t i = 0; i < 8; ++i)
    {
        value.digits[i] = prng_next_u64(ctx);
    }
    const size_t full_words = bits / 64;
    const size_t remaining_bits = bits % 64;
    if (remaining_bits != 0)
    {
        value.digits[full_words] &= (uint64_t(1) << remaining_bits) - 1;
    }
    const size_t first_unused_word = full_words + (remaining_bits != 0 ? 1 : 0);
    for (size_t i = first_unused_word; i < 8; ++i)
    {
        value.digits[i] = 0;
    }
    return value;
}

// Uniform rejection sampling in [0, limit-1].
uint512_t random_below(prng_ctx *ctx, const uint512_t &limit)
{
    if (limit.is_zero())
    {
        return uint512_t();
    }
    const size_t bits = limit.bit_length();
    uint512_t value;
    do
    {
        value = random_bits(ctx, bits);
    } while (value >= limit);
    return value;
}

bool divisible_by_small_prime(const uint512_t &n)
{
    for (size_t i = 0; i < sizeof(SMALL_PRIMES) / sizeof(SMALL_PRIMES[0]); ++i)
    {
        const uint16_t p = SMALL_PRIMES[i];
        if ((n % p) == 0)
        {
            return n != uint512_t(p);
        }
    }
    return false;
}

bool safe_prime_pair_passes_small_sieve(const uint512_t &q, const uint512_t &p)
{
    for (size_t i = 0; i < sizeof(SMALL_PRIMES) / sizeof(SMALL_PRIMES[0]); ++i)
    {
        const uint512_t small_prime(SMALL_PRIMES[i]);
        if ((q % SMALL_PRIMES[i]) == 0 && q != small_prime)
        {
            return false;
        }
        if ((p % SMALL_PRIMES[i]) == 0 && p != small_prime)
        {
            return false;
        }
    }
    return true;
}

bool miller_rabin_witness(const uint512_t &n, const uint512_t &d,
                          size_t twos, const uint512_t &base)
{
    uint512_t x = modular_exponentiation(base, d, n);
    const uint512_t one(1);
    const uint512_t n_minus_one = n - one;
    if (x == one || x == n_minus_one)
    {
        return false;
    }
    for (size_t r = 1; r < twos; ++r)
    {
        x = uint1024_t::mul_mod(x, x, n);
        if (x == n_minus_one)
        {
            return false;
        }
        if (x == one)
        {
            return true;
        }
    }
    return true;
}
}

bool is_probable_prime(const uint512_t &n, prng_ctx *ctx, size_t rounds)
{
    if (n < uint512_t(2))
    {
        return false;
    }
    if (n == uint512_t(2) || n == uint512_t(3))
    {
        return true;
    }
    if (n.is_even() || divisible_by_small_prime(n))
    {
        return false;
    }
    if (ctx == NULL || rounds == 0)
    {
        return false;
    }

    const uint512_t one(1);
    const uint512_t n_minus_one = n - one;
    uint512_t d = n_minus_one;
    size_t twos = 0;
    while (d.is_even())
    {
        d >>= 1;
        ++twos;
    }

    // Each base is independently uniform in [2, n-2]. For an odd composite,
    // the false-positive probability is at most 4^-rounds.
    const uint512_t base_range = n - uint512_t(3);
    for (size_t i = 0; i < rounds; ++i)
    {
        const uint512_t base = random_below(ctx, base_range) + uint512_t(2);
        if (miller_rabin_witness(n, d, twos, base))
        {
            return false;
        }
    }
    return true;
}

uint512_t generate_prime(prng_ctx *ctx, size_t bitsize, size_t rounds)
{
    if (ctx == NULL || bitsize < 2 || bitsize > 512 || rounds == 0)
    {
        return uint512_t();
    }
    for (;;)
    {
        uint512_t candidate = random_bits(ctx, bitsize);
        candidate.set_bit(bitsize - 1);
        candidate.set_bit(0);
        if (is_probable_prime(candidate, ctx, rounds))
        {
            return candidate;
        }
    }
}

uint512_t generate_safe_prime(prng_ctx *ctx, size_t bitsize)
{
    if (ctx == NULL || bitsize < 3 || bitsize > 512)
    {
        return uint512_t();
    }
    for (;;)
    {
        // q has bitsize-1 bits. Since q is odd, p=2q+1 is odd and bitsize bits.
        uint512_t q = random_bits(ctx, bitsize - 1);
        q.set_bit(bitsize - 2);
        q.set_bit(0);
        const uint512_t p = (q << 1) + uint512_t(1);
        if (p.bit_length() != bitsize || !safe_prime_pair_passes_small_sieve(q, p))
        {
            continue;
        }
        if (is_probable_prime(q, ctx, 40) && is_probable_prime(p, ctx, 40))
        {
            return p;
        }
    }
}
