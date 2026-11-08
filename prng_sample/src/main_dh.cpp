#include "dh_sample.h"

#include <cstring>
#include <iostream>

namespace
{
bool seed_from_system(prng_ctx &ctx)
{
    uint8_t seed[32] = {};
#if defined(_WIN32)
    HCRYPTPROV provider = 0;
    if (!CryptAcquireContext(&provider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        return false;
    }
    const bool success = CryptGenRandom(provider, sizeof(seed), seed) != 0;
    CryptReleaseContext(provider, 0);
    if (!success)
    {
        return false;
    }
#else
    size_t offset = 0;
    while (offset < sizeof(seed))
    {
        const ssize_t count = getrandom(seed + offset, sizeof(seed) - offset, 0);
        if (count < 0 && errno == EINTR)
        {
            continue;
        }
        if (count <= 0)
        {
            return false;
        }
        offset += static_cast<size_t>(count);
    }
#endif
    prng_seed(&ctx, seed, sizeof(seed));
    return true;
}
}

int main(int argc, char **argv)
{
    const bool test_mode = argc == 2 && std::strcmp(argv[1], "--test") == 0;
    if (argc > 1 && !test_mode)
    {
        std::cerr << "Usage: " << argv[0] << " [--test]\n";
        return 1;
    }

    prng_ctx ctx;
    if (test_mode)
    {
        const uint8_t seed[32] = {42};
        prng_seed(&ctx, seed, sizeof(seed));
        std::cout << "Test mode: fixed seed and small p=23 (learning only).\n";
    }
    else if (!seed_from_system(ctx))
    {
        std::cerr << "Cannot obtain system randomness.\n";
        return 1;
    }

    std::cout << "Generating safe prime parameters..." << std::endl;
    const uint512_t p = test_mode ? uint512_t(23) : generate_safe_prime(&ctx, 512);
    const uint512_t q = (p - uint512_t(1)) >> 1;
    if ((!test_mode && p.bit_length() != 512) ||
        !is_probable_prime(p, &ctx) || !is_probable_prime(q, &ctx))
    {
        std::cerr << "Invalid safe prime parameters.\n";
        return 1;
    }

    // For safe p=2q+1, a primitive root has neither order 1, 2 nor q.
    uint512_t g(2);
    const uint512_t upper = p - uint512_t(2);
    while (g <= upper &&
           (modular_exponentiation(g, uint512_t(2), p).is_one() ||
            modular_exponentiation(g, q, p).is_one()))
    {
        g += uint512_t(1);
    }
    if (g > upper)
    {
        std::cerr << "Cannot find a generator.\n";
        return 1;
    }

    const uint512_t alice_private = generate_private_key(&ctx, p);
    const uint512_t bob_private = generate_private_key(&ctx, p);
    if (alice_private < uint512_t(2) || alice_private > upper ||
        bob_private < uint512_t(2) || bob_private > upper)
    {
        std::cerr << "Invalid private key.\n";
        return 1;
    }

    const uint512_t alice_public = modular_exponentiation(g, alice_private, p);
    const uint512_t bob_public = modular_exponentiation(g, bob_private, p);
    if (alice_public < uint512_t(2) || alice_public > upper ||
        bob_public < uint512_t(2) || bob_public > upper)
    {
        std::cerr << "Invalid public key.\n";
        return 1;
    }

    // Simulate exchanging public keys; private keys stay with their owners.
    const uint512_t alice_secret = modular_exponentiation(bob_public, alice_private, p);
    const uint512_t bob_secret = modular_exponentiation(alice_public, bob_private, p);
    std::cout << "p = " << p << "\nq = " << q << "\ng = " << g
              << "\nAlice public key = " << alice_public
              << "\nBob public key = " << bob_public << "\n";
    if (alice_secret != bob_secret)
    {
        std::cerr << "Diffie-Hellman failed: shared secrets differ.\n";
        return 1;
    }
    if (test_mode)
    {
        std::cout << "Alice secret = " << alice_secret
                  << "\nBob secret = " << bob_secret << "\n";
    }
    std::cout << "Diffie-Hellman succeeded: shared secrets match.\n";
    return 0;
}
