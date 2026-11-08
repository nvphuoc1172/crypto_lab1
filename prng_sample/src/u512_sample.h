#ifndef U512_SAMPLE_H
#define U512_SAMPLE_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <iostream>
#include <string>

#if defined(__SIZEOF_INT128__) || (defined(__GNUC__) && defined(__x86_64__))
    #define HAS_INT128 1
    typedef unsigned __int128 uint128_t;
    typedef __int128 int128_t;
#endif

struct uint1024_t;

struct uint512_t
{
    // Little-endian: digits[0] holds bits 0..63, digits[7] holds bits 448..511
    uint64_t digits[8];

    // Constructors
    uint512_t();
    uint512_t(uint64_t val);

    // Bit-level helpers
    bool bit(size_t idx) const;
    void set_bit(size_t idx, bool val = true);
    size_t bit_length() const;

    // Numerical checks
    bool is_zero() const;
    bool is_one() const;
    bool is_odd() const;
    bool is_even() const;

    // String formatting
    std::string to_hex() const;
    // Throws std::invalid_argument for empty, malformed, or >512-bit input.
    static uint512_t from_hex(const std::string &hex_str);
    static uint512_t from_u64(uint64_t val);
};

// Arithmetic and relational operators for uint512_t
uint512_t operator+(const uint512_t &a, const uint512_t &b);
uint512_t operator-(const uint512_t &a, const uint512_t &b);
uint512_t operator<<(const uint512_t &a, size_t shift);
uint512_t operator>>(const uint512_t &a, size_t shift);

uint512_t &operator+=(uint512_t &a, const uint512_t &b);
uint512_t &operator-=(uint512_t &a, const uint512_t &b);
uint512_t &operator<<=(uint512_t &a, size_t shift);
uint512_t &operator>>=(uint512_t &a, size_t shift);

bool operator==(const uint512_t &a, const uint512_t &b);
bool operator!=(const uint512_t &a, const uint512_t &b);
bool operator<(const uint512_t &a, const uint512_t &b);
bool operator>(const uint512_t &a, const uint512_t &b);
bool operator>=(const uint512_t &a, const uint512_t &b);
bool operator<=(const uint512_t &a, const uint512_t &b);

// Fast modulo by 64-bit integer (useful for trial division with small primes)
uint64_t operator%(const uint512_t &a, uint64_t m);

// Stream output
std::ostream &operator<<(std::ostream &os, const uint512_t &val);

// Intermediate 1024-bit integer structure for multiplication and modulo
struct uint1024_t
{
    // Little-endian: digits[0] holds bits 0..63, digits[15] holds bits 960..1023
    uint64_t digits[16];

    uint1024_t();
    uint1024_t(uint64_t val);
    uint1024_t(const uint512_t &low);

    uint512_t to_u512() const;

    bool bit(size_t idx) const;
    void set_bit(size_t idx, bool val = true);
    size_t bit_length() const;
    bool is_zero() const;

    std::string to_hex() const;

    // Multiplication: 512-bit x 512-bit -> 1024-bit
    static uint1024_t mul(const uint512_t &a, const uint512_t &b);

    // Modulo: 1024-bit % 512-bit -> 512-bit
    static uint512_t mod(const uint1024_t &a, const uint512_t &b);

    // Arbitrary inputs are reduced; zero modulus throws std::invalid_argument.
    static uint512_t mul_mod(const uint512_t &a, const uint512_t &b, const uint512_t &mod);
    static uint512_t add_mod(const uint512_t &a, const uint512_t &b, const uint512_t &mod);
    static uint512_t sub_mod(const uint512_t &a, const uint512_t &b, const uint512_t &mod);
};

// Montgomery arithmetic context for odd 512-bit moduli. Values passed to
// multiply_montgomery() must already be in Montgomery representation.
class uint512_montgomery_t
{
public:
    explicit uint512_montgomery_t(const uint512_t &mod);
    bool is_valid() const;
    uint512_t to_montgomery(const uint512_t &value) const;
    uint512_t from_montgomery(const uint512_t &value) const;
    uint512_t multiply_montgomery(const uint512_t &a, const uint512_t &b) const;
    uint512_t multiply_mod(const uint512_t &a, const uint512_t &b) const;

private:
    uint512_t modulus_;
    uint512_t r_squared_;
    uint64_t negative_inverse_;
    bool valid_;
    uint512_t reduce_product(const uint1024_t &product) const;
};

// Operators for uint1024_t
uint1024_t operator+(const uint1024_t &a, const uint1024_t &b);
uint1024_t operator-(const uint1024_t &a, const uint1024_t &b);
uint1024_t operator<<(const uint1024_t &a, size_t shift);
uint1024_t operator>>(const uint1024_t &a, size_t shift);

uint1024_t &operator+=(uint1024_t &a, const uint1024_t &b);
uint1024_t &operator-=(uint1024_t &a, const uint1024_t &b);
uint1024_t &operator<<=(uint1024_t &a, size_t shift);
uint1024_t &operator>>=(uint1024_t &a, size_t shift);

bool operator==(const uint1024_t &a, const uint1024_t &b);
bool operator!=(const uint1024_t &a, const uint1024_t &b);
bool operator<(const uint1024_t &a, const uint1024_t &b);
bool operator>(const uint1024_t &a, const uint1024_t &b);
bool operator>=(const uint1024_t &a, const uint1024_t &b);
bool operator<=(const uint1024_t &a, const uint1024_t &b);

std::ostream &operator<<(std::ostream &os, const uint1024_t &val);

#endif
