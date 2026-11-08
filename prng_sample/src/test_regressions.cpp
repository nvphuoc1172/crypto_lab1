#include "dh_sample.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

int main()
{
    const uint512_t wide = uint512_t(1) << 64;
    assert(wide % UINT64_MAX == 1);
    assert(wide % uint64_t(7) == 2);
    assert(uint1024_t::add_mod(uint512_t(20), uint512_t(20), uint512_t(7)) == uint512_t(5));
    assert(uint1024_t::sub_mod(uint512_t(20), uint512_t(1), uint512_t(7)) == uint512_t(5));
    const char *bad[] = {"", "0x", "0x1g2"};
    for (const char *text : bad)
    {
        bool rejected = false;
        try { (void)uint512_t::from_hex(text); }
        catch (const std::invalid_argument &) { rejected = true; }
        assert(rejected);
    }
    bool rejected = false;
    try { (void)modular_exponentiation(uint512_t(2), uint512_t(3), uint512_t(0)); }
    catch (const std::invalid_argument &) { rejected = true; }
    assert(rejected);

    randombytes(NULL, 0);
    uint8_t guarded[34] = {};
    guarded[0] = 0x5a;
    guarded[33] = 0xa5;
    randombytes(guarded + 1, 32);
    assert(guarded[0] == 0x5a && guarded[33] == 0xa5);
    uint8_t seed[32] = {};
    uint8_t expected[14];
    shake128(expected, sizeof(expected), seed, sizeof(seed));
    prng_ctx ctx;
    prng_seed(&ctx, seed, sizeof(seed));
    const uint64_t values[] = {prng_next_u16(&ctx), prng_next_u32(&ctx), prng_next_u64(&ctx)};
    const size_t sizes[] = {2, 4, 8};
    size_t offset = 0;
    for (size_t n = 0; n < 3; ++n)
    {
        uint64_t value = 0;
        for (size_t i = 0; i < sizes[n]; ++i)
            value |= uint64_t(expected[offset++]) << (8 * i);
        assert(values[n] == value);
    }
    std::cout << "Regression tests passed.\n";
}
