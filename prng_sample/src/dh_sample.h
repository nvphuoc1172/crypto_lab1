#ifndef DH_H
#define DH_H

#include <stdint.h>
#include "prng.h"
#include "u512_sample.h"

uint512_t modular_exponentiation(const uint512_t &base, const uint512_t &exp, const uint512_t &mod);
bool is_probable_prime(const uint512_t &n, prng_ctx *ctx, size_t rounds = 40);
uint512_t generate_prime(prng_ctx *ctx, size_t bitsize = 512, size_t rounds = 40);
uint512_t generate_safe_prime(prng_ctx *ctx, size_t bitsize = 512);
// ctx must be seeded and p must be the agreed prime modulus.
// Uniform result in [2, p-2]; returns 0 for NULL ctx or p < 5.
uint512_t generate_private_key(prng_ctx *ctx, const uint512_t &p);

#endif
