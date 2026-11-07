#include "dh_sample.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include <cmath>
#include <chrono>

// Helper to initialize PRNG context with a deterministic seed for reproducibility
static void seed_context(prng_ctx &ctx, uint8_t marker)
{
    uint8_t seed[32];
    for (size_t i = 0; i < sizeof(seed); ++i)
    {
        seed[i] = static_cast<uint8_t>(marker + i);
    }
    prng_seed(&ctx, seed, sizeof(seed));
}

// ----------------------------------------------------------------------------
// Test 1: Validate handling of invalid input parameters
// ----------------------------------------------------------------------------
void test_invalid_parameters()
{
    std::cout << "[Test 1] Testing invalid parameters...\n";

    prng_ctx ctx;
    seed_context(ctx, 1);

    const uint512_t valid_p(1009);

    // 1. ctx == NULL must return 0
    assert(generate_private_key(NULL, valid_p).is_zero());

    // 2. p < 4 (range [2, p - 2] is invalid / degenerate) must return 0
    assert(generate_private_key(&ctx, uint512_t(0)).is_zero());
    assert(generate_private_key(&ctx, uint512_t(1)).is_zero());
    assert(generate_private_key(&ctx, uint512_t(2)).is_zero());
    assert(generate_private_key(&ctx, uint512_t(3)).is_zero());

    // 3. Overloaded function generate_private_key(p) with p < 4 must also return 0
    assert(generate_private_key(uint512_t(0)).is_zero());
    assert(generate_private_key(uint512_t(3)).is_zero());

    std::cout << "  -> PASSED: All invalid parameter cases correctly return 0.\n";
}

// ----------------------------------------------------------------------------
// Test 2: Check boundary values on small moduli
// ----------------------------------------------------------------------------
void test_small_boundaries()
{
    std::cout << "[Test 2] Testing boundary values with small moduli...\n";

    prng_ctx ctx;
    seed_context(ctx, 42);

    // When p = 4, range is [2, 4-2] = [2, 2], only 1 valid value exists (2)
    for (int i = 0; i < 20; ++i)
    {
        uint512_t key = generate_private_key(&ctx, uint512_t(4));
        assert(key == uint512_t(2));
    }

    // When p = 5, range is [2, 5-2] = [2, 3]
    bool seen_2 = false, seen_3 = false;
    for (int i = 0; i < 100; ++i)
    {
        uint512_t key = generate_private_key(&ctx, uint512_t(5));
        assert(key >= uint512_t(2) && key <= uint512_t(3));
        if (key == uint512_t(2)) seen_2 = true;
        if (key == uint512_t(3)) seen_3 = true;
    }
    assert(seen_2 && seen_3);

    // When p = 7, range is [2, 5]
    bool seen[4] = {false, false, false, false}; // for values 2, 3, 4, 5
    for (int i = 0; i < 200; ++i)
    {
        uint512_t key = generate_private_key(&ctx, uint512_t(7));
        assert(key >= uint512_t(2) && key <= uint512_t(5));
        size_t idx = static_cast<size_t>(key.digits[0] - 2);
        seen[idx] = true;
    }
    for (int i = 0; i < 4; ++i)
    {
        assert(seen[i]);
    }

    std::cout << "  -> PASSED: All generated keys strictly fall in [2, p - 2] with full coverage.\n";
}

// ----------------------------------------------------------------------------
// Test 3: Check uniform distribution (eliminating Modulo Bias)
// ----------------------------------------------------------------------------
void test_uniform_distribution()
{
    std::cout << "[Test 3] Testing uniform distribution (eliminating Modulo Bias)...\n";

    prng_ctx ctx;
    seed_context(ctx, 99);

    // For p = 7, values in [2, 5] are {2, 3, 4, 5} (4 distinct values)
    // Draw N = 20,000 samples, expected theoretical frequency is 5,000 (25%) each
    const int N = 20000;
    int count[4] = {0, 0, 0, 0};

    for (int i = 0; i < N; ++i)
    {
        uint512_t key = generate_private_key(&ctx, uint512_t(7));
        assert(key >= uint512_t(2) && key <= uint512_t(5));
        int val = static_cast<int>(key.digits[0]);
        count[val - 2]++;
    }

    std::cout << "  Statistics for 20,000 samples with p = 7 (range [2, 5]):\n";
    for (int i = 0; i < 4; ++i)
    {
        double percentage = (count[i] * 100.0) / N;
        std::cout << "    Value " << (i + 2) << ": " << count[i]
                  << " times (" << percentage << "%)\n";
        // Allow deviation within acceptable statistical tolerance (25% +- 3%)
        assert(count[i] >= 4400 && count[i] <= 5600);
    }

    std::cout << "  -> PASSED: Distribution is strictly uniform without modulo bias.\n";
}

// ----------------------------------------------------------------------------
// Test 4: Check 512-bit safe prime private key generation and overloading
// ----------------------------------------------------------------------------
void test_private_key_512bit()
{
    std::cout << "[Test 4] Testing 512-bit safe prime private key generation...\n";

    // 512-bit safe prime validated from prime generation module
    const uint512_t p = uint512_t::from_hex(
        "0x87345094e66da7782ed5ed9fe2d2c891cf4222938b8babbda2ee0d5ff262d9d2c873e174a40f107e76a2af9d3008e363945e9cbbdf5a9a082fd0cee5cc4f0a9f"
    );
    assert(p.bit_length() == 512);

    prng_ctx ctx_alice, ctx_bob;
    seed_context(ctx_alice, 10);
    seed_context(ctx_bob, 20);

    // Generate private keys for Alice and Bob
    uint512_t a = generate_private_key(&ctx_alice, p);
    uint512_t b = generate_private_key(&ctx_bob, p);

    const uint512_t two(2);
    const uint512_t p_minus_two = p - two;

    // Strict boundary condition: 2 <= a <= p - 2
    assert(a >= two);
    assert(a <= p_minus_two);
    assert(b >= two);
    assert(b <= p_minus_two);

    // Independent private keys must be distinct
    assert(a != b);

    std::cout << "  Alice private key a: " << a << "\n";
    std::cout << "  Bob private key b:   " << b << "\n";
    std::cout << "  Bit length of a:     " << a.bit_length() << " bits\n";
    std::cout << "  Bit length of b:     " << b.bit_length() << " bits\n";

    // Test overloaded function generate_private_key(p) using OS entropy
    uint512_t auto_key1 = generate_private_key(p);
    uint512_t auto_key2 = generate_private_key(p);
    assert(auto_key1 >= two && auto_key1 <= p_minus_two);
    assert(auto_key2 >= two && auto_key2 <= p_minus_two);
    assert(auto_key1 != auto_key2);

    std::cout << "  Overloaded generate_private_key(p) succeeded.\n";
    std::cout << "  -> PASSED: 512-bit private key generation valid.\n";
}

// ----------------------------------------------------------------------------
// Test 5: Full Diffie-Hellman key exchange simulation
// ----------------------------------------------------------------------------
void test_diffie_hellman_exchange()
{
    std::cout << "[Test 5] Simulating full Diffie-Hellman key exchange between Alice and Bob...\n";

    const uint512_t p = uint512_t::from_hex(
        "0x87345094e66da7782ed5ed9fe2d2c891cf4222938b8babbda2ee0d5ff262d9d2c873e174a40f107e76a2af9d3008e363945e9cbbdf5a9a082fd0cee5cc4f0a9f"
    );
    const uint512_t g(2); // Generator

    // Step 1: Generate private keys
    prng_ctx ctx_alice, ctx_bob;
    seed_context(ctx_alice, 77);
    seed_context(ctx_bob, 88);

    uint512_t a = generate_private_key(&ctx_alice, p); // Alice private key
    uint512_t b = generate_private_key(&ctx_bob, p);   // Bob private key

    // Step 2: Compute public keys
    // Alice computes A = g^a mod p
    uint512_t A = modular_exponentiation(g, a, p);
    // Bob computes B = g^b mod p
    uint512_t B = modular_exponentiation(g, b, p);

    assert(A > uint512_t(1) && A < p);
    assert(B > uint512_t(1) && B < p);

    // Step 3: Compute shared secrets
    // Alice receives B from Bob, computes s_A = B^a mod p
    uint512_t alice_shared_secret = modular_exponentiation(B, a, p);
    // Bob receives A from Alice, computes s_B = A^b mod p
    uint512_t bob_shared_secret = modular_exponentiation(A, b, p);

    // Step 4: Verify shared secrets match identically
    assert(alice_shared_secret == bob_shared_secret);
    assert(!alice_shared_secret.is_zero());
    assert(!alice_shared_secret.is_one());

    std::cout << "  Public key A (Alice): " << A << "\n";
    std::cout << "  Public key B (Bob):   " << B << "\n";
    std::cout << "  Shared secret Alice:  " << alice_shared_secret << "\n";
    std::cout << "  Shared secret Bob:    " << bob_shared_secret << "\n";
    std::cout << "  -> PASSED: Alice and Bob shared secrets MATCH IDENTICALLY!\n";
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    std::cout << "========================================================\n";
    std::cout << "  TEST SUITE: DIFFIE-HELLMAN PRIVATE KEY GENERATION    \n";
    std::cout << "========================================================\n";

    test_invalid_parameters();
    test_small_boundaries();
    test_uniform_distribution();
    test_private_key_512bit();
    test_diffie_hellman_exchange();

    std::cout << "========================================================\n";
    std::cout << "  ALL PRIVATE KEY GENERATION TESTS PASSED SUCCESSFULLY! \n";
    std::cout << "========================================================\n";

    return 0;
}
