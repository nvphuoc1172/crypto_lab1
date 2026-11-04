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

