#include "dh_sample.h"

#include <cassert>
#include <iostream>
#include <cstring>
#include <chrono>

static void seed_context(prng_ctx &ctx, uint8_t marker)
{
    uint8_t seed[32];
    for (size_t i = 0; i < sizeof(seed); ++i)
    {
        seed[i] = static_cast<uint8_t>(marker + i);
    }
    prng_seed(&ctx, seed, sizeof(seed));
}

int main(int argc, char **argv)
{
    prng_ctx ctx;
    if (argc > 1 && std::strcmp(argv[1], "--safe512") == 0)
    {
        seed_context(ctx, 101);
        const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        std::cerr << "Generating 512-bit safe prime..." << std::endl;
        const uint512_t p = generate_safe_prime(&ctx, 512);
        const uint512_t q = (p - uint512_t(1)) >> 1;
        assert(p.bit_length() == 512);
        assert(is_probable_prime(p, &ctx, 8));
        assert(is_probable_prime(q, &ctx, 8));
        const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
        std::cout << "512-bit safe prime test passed in " << elapsed.count()
                  << " seconds.\np=" << p << "\nq=" << q << "\n";
        return 0;
    }

    // Montgomery multiplication against hand-computed small results and a
    // 512-bit boundary identity: (n-1)^2 mod n == 1.
    assert(uint1024_t::mul_mod(uint512_t(123), uint512_t(456), uint512_t(1009)) == uint512_t(593));
    const uint512_t wide_mod = uint512_t::from_hex(
        "0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff43");
    const uint512_t wide_minus_one = wide_mod - uint512_t(1);
    const uint512_t wide_product = uint1024_t::mul_mod(wide_minus_one, wide_minus_one, wide_mod);
    if (wide_product != uint512_t(1))
    {
        std::cerr << "wide modulus bits=" << wide_mod.bit_length()
                  << " result=" << wide_product
                  << " reference=" << uint1024_t::mod(uint1024_t::mul(wide_minus_one, wide_minus_one), wide_mod)
                  << "\n";
        return 2;
    }

    for (uint64_t mod = 3; mod < 100; mod += 2)
    {
        for (uint64_t a = 0; a < 20; ++a)
        {
            for (uint64_t b = 0; b < 20; ++b)
            {
                assert(uint1024_t::mul_mod(uint512_t(a), uint512_t(b), uint512_t(mod)) ==
                       uint512_t((a * b) % mod));
            }
        }
    }

    for (uint64_t mod = 3; mod < 100; ++mod)
    {
        for (uint64_t base = 0; base < 10; ++base)
        {
            for (uint64_t exponent = 0; exponent < 12; ++exponent)
            {
                uint64_t expected = 1 % mod;
                for (uint64_t i = 0; i < exponent; ++i)
                {
                    expected = (expected * base) % mod;
                }
                assert(modular_exponentiation(uint512_t(base), uint512_t(exponent), uint512_t(mod)) ==
                       uint512_t(expected));
            }
        }
    }

    seed_context(ctx, 7);

    // Known small primes, including values that are in the trial-division list.
    const uint64_t primes[] = {2, 3, 5, 7, 97, 65537};
    for (size_t i = 0; i < sizeof(primes) / sizeof(primes[0]); ++i)
    {
        assert(is_probable_prime(uint512_t(primes[i]), &ctx, 12));
    }

    // Small composites, even values, and a Carmichael number.
    const uint64_t composites[] = {0, 1, 4, 9, 25, 341, 561, 1105, 1729};
    for (size_t i = 0; i < sizeof(composites) / sizeof(composites[0]); ++i)
    {
        assert(!is_probable_prime(uint512_t(composites[i]), &ctx, 12));
    }
    assert(!is_probable_prime(uint512_t(101ULL * 103ULL), &ctx, 12));
    assert(!is_probable_prime(uint512_t(1009ULL * 1013ULL), &ctx, 12));
    assert(!is_probable_prime(uint512_t(97), NULL, 12));
    assert(!is_probable_prime(uint512_t(97), &ctx, 0));

    seed_context(ctx, 29);
    const uint512_t prime = generate_prime(&ctx, 16, 12);
    assert(prime.bit_length() == 16);
    assert(prime.is_odd());
    assert(is_probable_prime(prime, &ctx, 20));
    assert(generate_prime(&ctx, 1, 12).is_zero());

    seed_context(ctx, 53);
    const uint512_t safe_prime = generate_safe_prime(&ctx, 5);
    assert(safe_prime.bit_length() == 5);
    assert(is_probable_prime(safe_prime, &ctx, 20));
    const uint512_t q = (safe_prime - uint512_t(1)) >> 1;
    assert(is_probable_prime(q, &ctx, 20));
    seed_context(ctx, 71);
    const uint512_t three_bit_safe_prime = generate_safe_prime(&ctx, 3);
    assert(three_bit_safe_prime == uint512_t(7)); // q=3 is itself in the sieve table.
    assert(generate_safe_prime(NULL, 512).is_zero());

    std::cout << "Prime tests passed.\n";
    std::cout << "16-bit probable prime: " << prime << "\n";
    std::cout << "5-bit safe prime: " << safe_prime << " (q=" << q << ")\n";
    return 0;
}
