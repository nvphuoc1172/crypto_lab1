#include "dh_sample.h"
#include <iostream>
#include <chrono>

int main()
{
    std::cout << "========================================================\n";
    std::cout << "    DIFFIE-HELLMAN KEY EXCHANGE DEMO (512-BIT)         \n";
    std::cout << "========================================================\n\n";

    // Initialize PRNG with cryptographically secure OS entropy
    prng_ctx ctx;
    uint8_t seed[32];
    randombytes(seed, sizeof(seed));
    prng_seed(&ctx, seed, sizeof(seed));

    // 1. Generate large safe prime p and choose generator g
    int bit_size = 512;
    std::cout << "[1] Generating a " << bit_size << "-bit safe prime (p = 2q + 1)...\n";
    const auto start_time = std::chrono::steady_clock::now();
    uint512_t p = generate_safe_prime(&ctx, bit_size);
    if (p.bit_length() != (size_t)bit_size)
    {
        std::cerr << "Safe prime generation failed.\n";
        return 1;
    }
    const auto end_time = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "  -> Generated successfully in " << elapsed.count() << " seconds.\n";
    std::cout << "  Safe prime p: " << p << "\n";
    std::cout << "  Bit length:   " << p.bit_length() << " bits\n\n";

    // For p=2q+1, exclude both proper subgroup orders (2 and q).
    const uint512_t q = (p - uint512_t(1)) >> 1;
    uint512_t g(2);
    while (g <= p - uint512_t(2) &&
           (modular_exponentiation(g, uint512_t(2), p).is_one() ||
            modular_exponentiation(g, q, p).is_one()))
        g += uint512_t(1);
    if (g > p - uint512_t(2))
        return 1;
    std::cout << "  Generator g:  " << g << "\n\n";

    // 2. Generate private keys for Alice and Bob in range [2, p - 2]
    std::cout << "[2] Generating private keys for Alice and Bob in [2, p - 2]...\n";
    uint512_t a = generate_private_key(&ctx, p); // Alice private key
    uint512_t b = generate_private_key(&ctx, p); // Bob private key
    if (a < uint512_t(2) || b < uint512_t(2) ||
        a > p - uint512_t(2) || b > p - uint512_t(2))
        return 1;

    std::cout << "  Alice private key (a): " << a << "\n";
    std::cout << "  Bob private key   (b): " << b << "\n\n";

    // 3. Compute public keys
    std::cout << "[3] Computing public keys for Alice and Bob...\n";
    uint512_t A = modular_exponentiation(g, a, p); // Alice computes A = g^a % p
    uint512_t B = modular_exponentiation(g, b, p); // Bob computes B = g^b % p

    std::cout << "  Alice public key (A = g^a mod p): " << A << "\n";
    std::cout << "  Bob public key   (B = g^b mod p): " << B << "\n\n";

    // 4. Compute shared secrets
    std::cout << "[4] Exchanging public keys and computing shared secret...\n";
    uint512_t alice_shared_secret = modular_exponentiation(B, a, p); // Alice computes s = B^a % p
    uint512_t bob_shared_secret = modular_exponentiation(A, b, p);   // Bob computes s = A^b % p

    std::cout << "  Alice shared secret: " << alice_shared_secret << "\n";
    std::cout << "  Bob shared secret:   " << bob_shared_secret << "\n\n";

    // 5. Display results and verify matching secrets
    std::cout << "[5] Verification:\n";
    bool match = (alice_shared_secret == bob_shared_secret);
    std::cout << "  Calculation correct? " << (match ? "1 (TRUE)" : "0 (FALSE)") << "\n";

    return match && !alice_shared_secret.is_zero() && !alice_shared_secret.is_one() ? 0 : 1;
}
