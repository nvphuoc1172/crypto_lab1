#include "dh_sample.h"

#include <cassert>
#include <iostream>

int main()
{
    const uint8_t seed[32] = {42};
    prng_ctx ctx;
    prng_ctx replay;
    prng_seed(&ctx, seed, sizeof(seed));
    prng_seed(&replay, seed, sizeof(seed));

    assert(generate_private_key(NULL, uint512_t(23)).is_zero());
    for (uint64_t p = 0; p < 5; ++p)
    {
        assert(generate_private_key(&ctx, uint512_t(p)).is_zero());
    }

    // Both endpoints must be reachable for the smallest supported prime.
    bool saw_two = false;
    bool saw_three = false;
    for (size_t i = 0; i < 128; ++i)
    {
        const uint512_t key = generate_private_key(&ctx, uint512_t(5));
        assert(key == generate_private_key(&replay, uint512_t(5)));
        assert(key >= uint512_t(2) && key <= uint512_t(3));
        saw_two = saw_two || key == uint512_t(2);
        saw_three = saw_three || key == uint512_t(3);
    }
    assert(saw_two && saw_three);

    // Cover partial limbs, exact limb boundaries and full 512-bit values.
    const size_t widths[] = {5, 64, 65, 128, 511, 512};
    for (size_t w = 0; w < sizeof(widths) / sizeof(widths[0]); ++w)
    {
        uint512_t p(0);
        for (size_t bit = 0; bit < widths[w]; ++bit)
        {
            p.set_bit(bit);
        }
        for (size_t i = 0; i < 128; ++i)
        {
            const uint512_t key = generate_private_key(&ctx, p);
            assert(key >= uint512_t(2) && key <= p - uint512_t(2));
        }
    }

    const uint512_t p(23);
    const uint512_t g(5);
    const uint512_t alice_private = generate_private_key(&ctx, p);
    const uint512_t bob_private = generate_private_key(&ctx, p);
    const uint512_t alice_public = modular_exponentiation(g, alice_private, p);
    const uint512_t bob_public = modular_exponentiation(g, bob_private, p);
    assert(modular_exponentiation(bob_public, alice_private, p) ==
           modular_exponentiation(alice_public, bob_private, p));
    std::cout << "Private key tests passed (including Alice/Bob shared secret).\n";
}
