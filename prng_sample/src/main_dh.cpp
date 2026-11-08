#include "dh_exchange.h"

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

    dh_exchange_result exchange;
    if (!perform_dh_exchange(&ctx, p, g, exchange))
    {
        std::cerr << "Cannot complete a non-degenerate DH exchange within 128 attempts.\n";
        return 1;
    }
    std::cout << "p = " << p << "\nq = " << q << "\ng = " << g
              << "\nAlice public key = " << exchange.alice_public
              << "\nBob public key = " << exchange.bob_public << "\n";
    if (test_mode)
    {
        std::cout << "Alice secret = " << exchange.alice_secret
                  << "\nBob secret = " << exchange.bob_secret << "\n";
    }
    std::cout << "Diffie-Hellman succeeded: shared secrets match.\n";
    return 0;
}
