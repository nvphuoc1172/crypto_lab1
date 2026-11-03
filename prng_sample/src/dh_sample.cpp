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

    uint512_t result(1);
    uint512_t cur = uint1024_t::mod(uint1024_t(base), mod);
    size_t exp_len = exp.bit_length();

    if (exp_len == 0)
    {
        return uint512_t(1);
    }

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

