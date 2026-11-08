#include "u512_sample.h"
#include <iomanip>
#include <sstream>
#include <stdexcept>

#if defined(_MSC_VER)
    #include <intrin.h>
#endif

// ============================================================================
// uint512_t implementation
// ============================================================================

uint512_t::uint512_t()
{
    for (size_t i = 0; i < 8; i++)
    {
        digits[i] = 0;
    }
}

uint512_t::uint512_t(uint64_t val)
{
    digits[0] = val;
    for (size_t i = 1; i < 8; i++)
    {
        digits[i] = 0;
    }
}

uint512_t uint512_t::from_u64(uint64_t val)
{
    return uint512_t(val);
}

bool uint512_t::bit(size_t idx) const
{
    if (idx >= 512)
    {
        return false;
    }
    return (digits[idx / 64] >> (idx % 64)) & 1ULL;
}

void uint512_t::set_bit(size_t idx, bool val)
{
    if (idx >= 512)
    {
        return;
    }
    if (val)
    {
        digits[idx / 64] |= (1ULL << (idx % 64));
    }
    else
    {
        digits[idx / 64] &= ~(1ULL << (idx % 64));
    }
}

size_t uint512_t::bit_length() const
{
    for (int i = 7; i >= 0; i--)
    {
        if (digits[i] != 0)
        {
            uint64_t w = digits[i];
            size_t bits = 0;
            while (w > 0)
            {
                bits++;
                w >>= 1;
            }
            return (size_t)i * 64 + bits;
        }
    }
    return 0;
}

bool uint512_t::is_zero() const
{
    for (size_t i = 0; i < 8; i++)
    {
        if (digits[i] != 0)
        {
            return false;
        }
    }
    return true;
}

bool uint512_t::is_one() const
{
    if (digits[0] != 1)
    {
        return false;
    }
    for (size_t i = 1; i < 8; i++)
    {
        if (digits[i] != 0)
        {
            return false;
        }
    }
    return true;
}

bool uint512_t::is_odd() const
{
    return (digits[0] & 1ULL) != 0;
}

bool uint512_t::is_even() const
{
    return (digits[0] & 1ULL) == 0;
}

std::string uint512_t::to_hex() const
{
    if (is_zero())
    {
        return "0x0";
    }
    std::ostringstream oss;
    oss << "0x";
    int highest = 7;
    while (highest > 0 && digits[highest] == 0)
    {
        highest--;
    }
    oss << std::hex << digits[highest];
    for (int i = highest - 1; i >= 0; i--)
    {
        oss << std::hex << std::setfill('0') << std::setw(16) << digits[i];
    }
    return oss.str();
}

uint512_t uint512_t::from_hex(const std::string &hex_str)
{
    uint512_t res;
    size_t start = 0;
    if (hex_str.size() >= 2 && hex_str[0] == '0' && (hex_str[1] == 'x' || hex_str[1] == 'X'))
    {
        start = 2;
    }

    if (hex_str.size() == start || hex_str.size() - start > 128)
        throw std::invalid_argument("Expected 1 to 128 hexadecimal digits");
    size_t bit_pos = 0;
    for (int i = (int)hex_str.size() - 1; i >= (int)start && bit_pos < 512; i--)
    {
        char c = hex_str[i];
        uint8_t val = 0;
        if (c >= '0' && c <= '9')
        {
            val = c - '0';
        }
        else if (c >= 'a' && c <= 'f')
        {
            val = c - 'a' + 10;
        }
        else if (c >= 'A' && c <= 'F')
        {
            val = c - 'A' + 10;
        }
        else
        {
            throw std::invalid_argument("Invalid hexadecimal digit");
        }

        for (int b = 0; b < 4 && bit_pos < 512; b++)
        {
            if ((val >> b) & 1)
            {
                res.set_bit(bit_pos, true);
            }
            bit_pos++;
        }
    }
    return res;
}

// ----------------------------------------------------------------------------
// uint512_t Operators
// ----------------------------------------------------------------------------

uint512_t operator+(const uint512_t &a, const uint512_t &b)
{
    uint512_t res;
    uint64_t carry = 0;
    for (size_t i = 0; i < 8; i++)
    {
#if defined(HAS_INT128)
        uint128_t sum = (uint128_t)a.digits[i] + b.digits[i] + carry;
        res.digits[i] = (uint64_t)sum;
        carry = (uint64_t)(sum >> 64);
#else
        uint64_t sum = a.digits[i] + carry;
        uint64_t c1 = (sum < carry) ? 1 : 0;
        sum += b.digits[i];
        uint64_t c2 = (sum < b.digits[i]) ? 1 : 0;
        res.digits[i] = sum;
        carry = c1 + c2;
#endif
    }
    return res;
}

uint512_t operator-(const uint512_t &a, const uint512_t &b)
{
    uint512_t res;
    uint64_t borrow = 0;
    for (size_t i = 0; i < 8; i++)
    {
#if defined(HAS_INT128)
        int128_t diff = (int128_t)a.digits[i] - b.digits[i] - borrow;
        res.digits[i] = (uint64_t)diff;
        borrow = (diff < 0) ? 1 : 0;
#else
        uint64_t sub1 = a.digits[i] - borrow;
        uint64_t b1 = (a.digits[i] < borrow) ? 1 : 0;
        uint64_t sub2 = sub1 - b.digits[i];
        uint64_t b2 = (sub1 < b.digits[i]) ? 1 : 0;
        res.digits[i] = sub2;
        borrow = b1 + b2;
#endif
    }
    return res;
}

uint512_t operator<<(const uint512_t &a, size_t shift)
{
    if (shift >= 512)
    {
        return uint512_t();
    }
    size_t word_shift = shift / 64;
    size_t bit_shift = shift % 64;

    uint512_t res;
    for (size_t i = 0; i < 8; i++)
    {
        if (i >= word_shift)
        {
            res.digits[i] = a.digits[i - word_shift] << bit_shift;
            if (bit_shift > 0 && i > word_shift)
            {
                res.digits[i] |= (a.digits[i - word_shift - 1] >> (64 - bit_shift));
            }
        }
    }
    return res;
}

uint512_t operator>>(const uint512_t &a, size_t shift)
{
    if (shift >= 512)
    {
        return uint512_t();
    }
    size_t word_shift = shift / 64;
    size_t bit_shift = shift % 64;

    uint512_t res;
    for (size_t i = 0; i < 8; i++)
    {
        if (i + word_shift < 8)
        {
            res.digits[i] = a.digits[i + word_shift] >> bit_shift;
            if (bit_shift > 0 && i + word_shift + 1 < 8)
            {
                res.digits[i] |= (a.digits[i + word_shift + 1] << (64 - bit_shift));
            }
        }
    }
    return res;
}

uint512_t &operator+=(uint512_t &a, const uint512_t &b)
{
    a = a + b;
    return a;
}

uint512_t &operator-=(uint512_t &a, const uint512_t &b)
{
    a = a - b;
    return a;
}

uint512_t &operator<<=(uint512_t &a, size_t shift)
{
    a = a << shift;
    return a;
}

uint512_t &operator>>=(uint512_t &a, size_t shift)
{
    a = a >> shift;
    return a;
}

bool operator==(const uint512_t &a, const uint512_t &b)
{
    for (size_t i = 0; i < 8; i++)
    {
        if (a.digits[i] != b.digits[i])
        {
            return false;
        }
    }
    return true;
}

bool operator!=(const uint512_t &a, const uint512_t &b)
{
    return !(a == b);
}

bool operator<(const uint512_t &a, const uint512_t &b)
{
    for (int i = 7; i >= 0; i--)
    {
        if (a.digits[i] < b.digits[i])
        {
            return true;
        }
        if (a.digits[i] > b.digits[i])
        {
            return false;
        }
    }
    return false;
}

bool operator>(const uint512_t &a, const uint512_t &b)
{
    return b < a;
}

bool operator>=(const uint512_t &a, const uint512_t &b)
{
    return !(a < b);
}

bool operator<=(const uint512_t &a, const uint512_t &b)
{
    return !(b < a);
}

uint64_t operator%(const uint512_t &a, uint64_t m)
{
    if (m == 0)
    {
        throw std::invalid_argument("Modulus must be nonzero");
    }
    uint64_t rem = 0;
    for (int i = 7; i >= 0; i--)
    {
#if defined(HAS_INT128)
        uint128_t cur = ((uint128_t)rem << 64) | a.digits[i];
        rem = (uint64_t)(cur % m);
#else
        if (m <= UINT32_MAX)
        {
            rem = ((rem << 32) | (a.digits[i] >> 32)) % m;
            rem = ((rem << 32) | (a.digits[i] & UINT32_MAX)) % m;
            continue;
        }
        for (int bit = 63; bit >= 0; --bit)
        {
            // Double without overflowing: rem is always less than m.
            rem = rem >= m - rem ? rem - (m - rem) : rem + rem;
            if ((a.digits[i] >> bit) & 1ULL)
                rem = rem == m - 1 ? 0 : rem + 1;
        }
#endif
    }
    return rem;
}

std::ostream &operator<<(std::ostream &os, const uint512_t &val)
{
    os << val.to_hex();
    return os;
}

// ============================================================================
// uint1024_t implementation
// ============================================================================

uint1024_t::uint1024_t()
{
    for (size_t i = 0; i < 16; i++)
    {
        digits[i] = 0;
    }
}

uint1024_t::uint1024_t(uint64_t val)
{
    digits[0] = val;
    for (size_t i = 1; i < 16; i++)
    {
        digits[i] = 0;
    }
}

uint1024_t::uint1024_t(const uint512_t &low)
{
    for (size_t i = 0; i < 8; i++)
    {
        digits[i] = low.digits[i];
    }
    for (size_t i = 8; i < 16; i++)
    {
        digits[i] = 0;
    }
}

uint512_t uint1024_t::to_u512() const
{
    uint512_t res;
    for (size_t i = 0; i < 8; i++)
    {
        res.digits[i] = digits[i];
    }
    return res;
}

bool uint1024_t::bit(size_t idx) const
{
    if (idx >= 1024)
    {
        return false;
    }
    return (digits[idx / 64] >> (idx % 64)) & 1ULL;
}

void uint1024_t::set_bit(size_t idx, bool val)
{
    if (idx >= 1024)
    {
        return;
    }
    if (val)
    {
        digits[idx / 64] |= (1ULL << (idx % 64));
    }
    else
    {
        digits[idx / 64] &= ~(1ULL << (idx % 64));
    }
}

size_t uint1024_t::bit_length() const
{
    for (int i = 15; i >= 0; i--)
    {
        if (digits[i] != 0)
        {
            uint64_t w = digits[i];
            size_t bits = 0;
            while (w > 0)
            {
                bits++;
                w >>= 1;
            }
            return (size_t)i * 64 + bits;
        }
    }
    return 0;
}

bool uint1024_t::is_zero() const
{
    for (size_t i = 0; i < 16; i++)
    {
        if (digits[i] != 0)
        {
            return false;
        }
    }
    return true;
}

std::string uint1024_t::to_hex() const
{
    if (is_zero())
    {
        return "0x0";
    }
    std::ostringstream oss;
    oss << "0x";
    int highest = 15;
    while (highest > 0 && digits[highest] == 0)
    {
        highest--;
    }
    oss << std::hex << digits[highest];
    for (int i = highest - 1; i >= 0; i--)
    {
        oss << std::hex << std::setfill('0') << std::setw(16) << digits[i];
    }
    return oss.str();
}

uint1024_t operator+(const uint1024_t &a, const uint1024_t &b)
{
    uint1024_t res;
    uint64_t carry = 0;
    for (size_t i = 0; i < 16; i++)
    {
#if defined(HAS_INT128)
        uint128_t sum = (uint128_t)a.digits[i] + b.digits[i] + carry;
        res.digits[i] = (uint64_t)sum;
        carry = (uint64_t)(sum >> 64);
#else
        uint64_t sum = a.digits[i] + carry;
        uint64_t c1 = (sum < carry) ? 1 : 0;
        sum += b.digits[i];
        uint64_t c2 = (sum < b.digits[i]) ? 1 : 0;
        res.digits[i] = sum;
        carry = c1 + c2;
#endif
    }
    return res;
}

uint1024_t operator-(const uint1024_t &a, const uint1024_t &b)
{
    uint1024_t res;
    uint64_t borrow = 0;
    for (size_t i = 0; i < 16; i++)
    {
#if defined(HAS_INT128)
        int128_t diff = (int128_t)a.digits[i] - b.digits[i] - borrow;
        res.digits[i] = (uint64_t)diff;
        borrow = (diff < 0) ? 1 : 0;
#else
        uint64_t sub1 = a.digits[i] - borrow;
        uint64_t b1 = (a.digits[i] < borrow) ? 1 : 0;
        uint64_t sub2 = sub1 - b.digits[i];
        uint64_t b2 = (sub1 < b.digits[i]) ? 1 : 0;
        res.digits[i] = sub2;
        borrow = b1 + b2;
#endif
    }
    return res;
}

uint1024_t operator<<(const uint1024_t &a, size_t shift)
{
    if (shift >= 1024)
    {
        return uint1024_t();
    }
    size_t word_shift = shift / 64;
    size_t bit_shift = shift % 64;

    uint1024_t res;
    for (size_t i = 0; i < 16; i++)
    {
        if (i >= word_shift)
        {
            res.digits[i] = a.digits[i - word_shift] << bit_shift;
            if (bit_shift > 0 && i > word_shift)
            {
                res.digits[i] |= (a.digits[i - word_shift - 1] >> (64 - bit_shift));
            }
        }
    }
    return res;
}

uint1024_t operator>>(const uint1024_t &a, size_t shift)
{
    if (shift >= 1024)
    {
        return uint1024_t();
    }
    size_t word_shift = shift / 64;
    size_t bit_shift = shift % 64;

    uint1024_t res;
    for (size_t i = 0; i < 16; i++)
    {
        if (i + word_shift < 16)
        {
            res.digits[i] = a.digits[i + word_shift] >> bit_shift;
            if (bit_shift > 0 && i + word_shift + 1 < 16)
            {
                res.digits[i] |= (a.digits[i + word_shift + 1] << (64 - bit_shift));
            }
        }
    }
    return res;
}

uint1024_t &operator+=(uint1024_t &a, const uint1024_t &b)
{
    a = a + b;
    return a;
}

uint1024_t &operator-=(uint1024_t &a, const uint1024_t &b)
{
    a = a - b;
    return a;
}

uint1024_t &operator<<=(uint1024_t &a, size_t shift)
{
    a = a << shift;
    return a;
}

uint1024_t &operator>>=(uint1024_t &a, size_t shift)
{
    a = a >> shift;
    return a;
}

bool operator==(const uint1024_t &a, const uint1024_t &b)
{
    for (size_t i = 0; i < 16; i++)
    {
        if (a.digits[i] != b.digits[i])
        {
            return false;
        }
    }
    return true;
}

bool operator!=(const uint1024_t &a, const uint1024_t &b)
{
    return !(a == b);
}

bool operator<(const uint1024_t &a, const uint1024_t &b)
{
    for (int i = 15; i >= 0; i--)
    {
        if (a.digits[i] < b.digits[i])
        {
            return true;
        }
        if (a.digits[i] > b.digits[i])
        {
            return false;
        }
    }
    return false;
}

bool operator>(const uint1024_t &a, const uint1024_t &b)
{
    return b < a;
}

bool operator>=(const uint1024_t &a, const uint1024_t &b)
{
    return !(a < b);
}

bool operator<=(const uint1024_t &a, const uint1024_t &b)
{
    return !(b < a);
}

std::ostream &operator<<(std::ostream &os, const uint1024_t &val)
{
    os << val.to_hex();
    return os;
}

// ----------------------------------------------------------------------------
// Static Multi-Precision & Modular Operations
// ----------------------------------------------------------------------------

uint1024_t uint1024_t::mul(const uint512_t &a, const uint512_t &b)
{
    uint1024_t res;
#if !defined(HAS_INT128)
    // Portable 32-bit schoolbook multiplication. Each product plus the
    // destination word and carry fits exactly in uint64_t.
    uint32_t a_words[16];
    uint32_t b_words[16];
    uint32_t out_words[32] = {0};
    for (size_t i = 0; i < 8; ++i)
    {
        a_words[2 * i] = (uint32_t)a.digits[i];
        a_words[2 * i + 1] = (uint32_t)(a.digits[i] >> 32);
        b_words[2 * i] = (uint32_t)b.digits[i];
        b_words[2 * i + 1] = (uint32_t)(b.digits[i] >> 32);
    }
    for (size_t i = 0; i < 16; ++i)
    {
        uint64_t carry = 0;
        for (size_t j = 0; j < 16; ++j)
        {
            const uint64_t sum = (uint64_t)a_words[i] * b_words[j] +
                                 out_words[i + j] + carry;
            out_words[i + j] = (uint32_t)sum;
            carry = sum >> 32;
        }
        size_t k = i + 16;
        while (carry != 0 && k < 32)
        {
            const uint64_t sum = (uint64_t)out_words[k] + carry;
            out_words[k] = (uint32_t)sum;
            carry = sum >> 32;
            ++k;
        }
    }
    for (size_t i = 0; i < 16; ++i)
    {
        res.digits[i] = (uint64_t)out_words[2 * i] |
                        ((uint64_t)out_words[2 * i + 1] << 32);
    }
    return res;
#else
    for (size_t i = 0; i < 8; i++)
    {
        if (a.digits[i] == 0)
        {
            continue;
        }
        uint64_t carry = 0;
        for (size_t j = 0; j < 8; j++)
        {
            uint128_t prod = (uint128_t)a.digits[i] * b.digits[j] + res.digits[i + j] + carry;
            res.digits[i + j] = (uint64_t)prod;
            carry = (uint64_t)(prod >> 64);
        }
        size_t k = i + 8;
        while (carry > 0 && k < 16)
        {
            uint64_t sum = res.digits[k] + carry;
            carry = (sum < res.digits[k]) ? 1 : 0;
            res.digits[k] = sum;
            k++;
        }
    }
    return res;
#endif
}

uint512_t uint1024_t::mod(const uint1024_t &a, const uint512_t &b)
{
    if (b.is_zero())
    {
        throw std::invalid_argument("Modulus must be nonzero");
    }
    uint1024_t rem;
    uint1024_t b_ext(b);
    size_t a_len = a.bit_length();
    for (int i = (int)a_len - 1; i >= 0; i--)
    {
        rem <<= 1;
        if (a.bit(i))
        {
            rem.digits[0] |= 1;
        }
        if (rem >= b_ext)
        {
            rem -= b_ext;
        }
    }
    return rem.to_u512();
}

uint512_t uint1024_t::mul_mod(const uint512_t &a, const uint512_t &b, const uint512_t &mod)
{
    if (mod.is_odd() && !mod.is_one())
    {
        uint512_montgomery_t montgomery(mod);
        if (montgomery.is_valid())
        {
            return montgomery.multiply_mod(a, b);
        }
    }
    uint1024_t prod = uint1024_t::mul(a, b);
    return uint1024_t::mod(prod, mod);
}

uint512_montgomery_t::uint512_montgomery_t(const uint512_t &mod)
    : modulus_(mod), r_squared_(), negative_inverse_(0), valid_(false)
{
    if (!mod.is_odd() || mod.is_one() || mod.is_zero())
    {
        return;
    }

    // Newton iteration computes mod[0]^-1 modulo 2^32; negate it for REDC.
    uint32_t inverse = 1;
    for (size_t i = 0; i < 5; ++i)
    {
        inverse *= uint32_t(2) - (uint32_t)mod.digits[0] * inverse;
    }
    negative_inverse_ = uint32_t(0) - inverse;

    // R^2 mod n, where R=2^512. This setup is done once per exponentiation.
    r_squared_ = uint512_t(1);
    for (size_t i = 0; i < 1024; ++i)
    {
        r_squared_ = uint1024_t::add_mod(r_squared_, r_squared_, modulus_);
    }
    valid_ = true;
}

bool uint512_montgomery_t::is_valid() const
{
    return valid_;
}

uint512_t uint512_montgomery_t::reduce_product(const uint1024_t &product) const
{
    // Use radix 2^32 so this reduction works identically on compilers with
    // and without a native 128-bit integer type.
    uint32_t n[16];
    uint32_t t[33] = {0};
    for (size_t i = 0; i < 8; ++i)
    {
        n[2 * i] = (uint32_t)modulus_.digits[i];
        n[2 * i + 1] = (uint32_t)(modulus_.digits[i] >> 32);
    }
    for (size_t i = 0; i < 16; ++i)
    {
        t[2 * i] = (uint32_t)product.digits[i];
        t[2 * i + 1] = (uint32_t)(product.digits[i] >> 32);
    }

    for (size_t i = 0; i < 16; ++i)
    {
        const uint32_t m = t[i] * (uint32_t)negative_inverse_;
        uint64_t carry = 0;
        for (size_t j = 0; j < 16; ++j)
        {
            const uint64_t sum = (uint64_t)m * n[j] + t[i + j] + carry;
            t[i + j] = (uint32_t)sum;
            carry = sum >> 32;
        }

        size_t k = i + 16;
        while (carry != 0 && k < 33)
        {
            const uint64_t sum = (uint64_t)t[k] + carry;
            t[k] = (uint32_t)sum;
            carry = sum >> 32;
            ++k;
        }
    }

    uint512_t result;
    for (size_t i = 0; i < 16; ++i)
    {
        result.digits[i / 2] |= (uint64_t)t[i + 16] << ((i % 2) * 32);
    }

    if (t[32] != 0 || result >= modulus_)
    {
        uint64_t borrow = 0;
        for (size_t i = 0; i < 8; ++i)
        {
            const uint64_t old = result.digits[i];
            const uint64_t after_borrow = old - borrow;
            const uint64_t borrow1 = old < borrow ? 1 : 0;
            const uint64_t value = after_borrow - modulus_.digits[i];
            const uint64_t borrow2 = after_borrow < modulus_.digits[i] ? 1 : 0;
            result.digits[i] = value;
            borrow = borrow1 | borrow2;
        }
        // A ninth limb can only be one; subtracting n consumes the borrow.
    }
    return result;
}

uint512_t uint512_montgomery_t::multiply_montgomery(const uint512_t &a, const uint512_t &b) const
{
    if (!valid_)
    {
        return uint512_t();
    }
    return reduce_product(uint1024_t::mul(a, b));
}

uint512_t uint512_montgomery_t::to_montgomery(const uint512_t &value) const
{
    if (!valid_)
    {
        return uint512_t();
    }
    const uint512_t reduced = uint1024_t::mod(uint1024_t(value), modulus_);
    return multiply_montgomery(reduced, r_squared_);
}

uint512_t uint512_montgomery_t::from_montgomery(const uint512_t &value) const
{
    if (!valid_)
    {
        return uint512_t();
    }
    return multiply_montgomery(value, uint512_t(1));
}

uint512_t uint512_montgomery_t::multiply_mod(const uint512_t &a, const uint512_t &b) const
{
    if (!valid_)
    {
        return uint512_t();
    }
    return from_montgomery(multiply_montgomery(to_montgomery(a), to_montgomery(b)));
}

uint512_t uint1024_t::add_mod(const uint512_t &a, const uint512_t &b, const uint512_t &mod)
{
    if (mod.is_zero())
        throw std::invalid_argument("Modulus must be nonzero");
    if (a >= mod || b >= mod)
        return uint1024_t::mod(uint1024_t(a) + uint1024_t(b), mod);
    uint1024_t sum = uint1024_t(a) + uint1024_t(b);
    if (sum >= uint1024_t(mod))
    {
        sum -= uint1024_t(mod);
    }
    return sum.to_u512();
}

uint512_t uint1024_t::sub_mod(const uint512_t &a, const uint512_t &b, const uint512_t &mod)
{
    if (mod.is_zero())
        throw std::invalid_argument("Modulus must be nonzero");
    if (a >= mod || b >= mod)
        return sub_mod(uint1024_t::mod(uint1024_t(a), mod),
                       uint1024_t::mod(uint1024_t(b), mod), mod);
    if (a >= b)
    {
        return a - b;
    }
    return (a + mod) - b;
}

