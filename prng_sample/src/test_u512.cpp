#include "u512_sample.h"
#include "dh_sample.h"
#include <iostream>
#include <cassert>

void test_basic_arithmetic()
{
    std::cout << "[Test 1] Testing basic arithmetic...\n";

    // 0 + 0 = 0
    uint512_t z1, z2;
    uint512_t z3 = z1 + z2;
    assert(z3.is_zero());

    // 1 + 1 = 2
    uint512_t one(1);
    uint512_t two = one + one;
    assert(two.digits[0] == 2);
    assert(two.bit_length() == 2);

    // Carry across limb boundary: 0xFFFFFFFFFFFFFFFF + 1 = 0x1 0000000000000000
    uint512_t max_u64;
    max_u64.digits[0] = 0xFFFFFFFFFFFFFFFFULL;
    uint512_t carry_test = max_u64 + one;
    assert(carry_test.digits[0] == 0);
    assert(carry_test.digits[1] == 1);

    // Subtraction borrow: carry_test - 1 = max_u64
    uint512_t sub_test = carry_test - one;
    assert(sub_test.digits[0] == 0xFFFFFFFFFFFFFFFFULL);
    assert(sub_test.digits[1] == 0);

    // Comparison tests
    assert(one < two);
    assert(two > one);
    assert(one <= two);
    assert(two >= one);
    assert(one == one);
    assert(one != two);

    std::cout << "  -> Basic arithmetic PASSED!\n";
}

void test_shifts()
{
    std::cout << "[Test 2] Testing shift operations...\n";

    uint512_t one(1);
    // Shift by 64 bits (crosses limb boundary)
    uint512_t s64 = one << 64;
    assert(s64.digits[0] == 0);
    assert(s64.digits[1] == 1);
    assert(s64.bit_length() == 65);

    // Shift right
    uint512_t r64 = s64 >> 64;
    assert(r64 == one);

    // Arbitrary shift across boundary (e.g. 70 bits)
    uint512_t s70 = one << 70;
    assert(s70.digits[0] == 0);
    assert(s70.digits[1] == (1ULL << 6));
    assert((s70 >> 70) == one);

    std::cout << "  -> Shift operations PASSED!\n";
}

void test_hex_conversion()
{
    std::cout << "[Test 3] Testing hex conversions...\n";

    std::string hex = "0x123456789abcdef0123456789abcdef0";
    uint512_t val = uint512_t::from_hex(hex);
    std::string back_hex = val.to_hex();
    assert(back_hex == hex);

    std::cout << "  -> Hex conversion PASSED!\n";
}

void test_multiplication_and_modulo()
{
    std::cout << "[Test 4] Testing 512x512 multiplication and modulo...\n";

    // 0xFFFFFFFFFFFFFFFF * 0xFFFFFFFFFFFFFFFF
    uint512_t a(0xFFFFFFFFFFFFFFFFULL);
    uint512_t b(0xFFFFFFFFFFFFFFFFULL);
    uint1024_t prod = uint1024_t::mul(a, b);

    // Expected: 0xFFFFFFFFFFFFFFFE0000000000000001
    // low 64 bits = 1, high 64 bits = 0xFFFFFFFFFFFFFFFE
    assert(prod.digits[0] == 1);
    assert(prod.digits[1] == 0xFFFFFFFFFFFFFFFEULL);

    // Modulo test: prod % a == 0
    uint512_t rem = uint1024_t::mod(prod, a);
    assert(rem.is_zero());

    // Small prime modulo operator%
    uint512_t val = uint512_t::from_hex("0x64"); // 100 in hex
    assert((val % 7) == (100 % 7));
    assert((val % 13) == (100 % 13));

    std::cout << "  -> Multiplication and modulo PASSED!\n";
}

void test_modular_exponentiation()
{
    std::cout << "[Test 5] Testing modular exponentiation...\n";

    // Small known values: 3^10 % 1000 = 59049 % 1000 = 49
    uint512_t base(3);
    uint512_t exp(10);
    uint512_t mod(1000);
    uint512_t res = modular_exponentiation(base, exp, mod);
    assert(res.digits[0] == 49);

    // Fermat's Little Theorem test: 2^(p-1) % p = 1 for prime p = 65537
    uint512_t p(65537);
    uint512_t p_minus_1(65536);
    uint512_t flt = modular_exponentiation(uint512_t(2), p_minus_1, p);
    assert(flt.is_one());

    std::cout << "  -> Modular exponentiation PASSED!\n";
}

int main()
{
    std::cout << "=== RUNNING BIGINT UNIT TESTS ===\n";
    test_basic_arithmetic();
    test_shifts();
    test_hex_conversion();
    test_multiplication_and_modulo();
    test_modular_exponentiation();
    std::cout << "=== ALL BIGINT TESTS PASSED SUCCESSFULLY! ===\n";
    return 0;
}

