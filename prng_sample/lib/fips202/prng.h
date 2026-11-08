#ifndef PRNG_H
#define PRNG_H
#include "fips202.h"
#include <stdlib.h>
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincrypt.h>
#elif defined(__linux__)
#include <sys/random.h>
#else
#error "OS entropy is implemented for Windows and Linux only"
#endif
#include <errno.h>

typedef struct
{
    keccak_state sponge;
} prng_ctx;

// At most 255 bytes; aborts on OS entropy failure. Zero length accepts NULL.
void randombytes(uint8_t *out, uint8_t outlen);
void prng_seed(prng_ctx *ctx, const uint8_t *seed, size_t seedlen);
uint16_t prng_next_u16(prng_ctx *ctx);
uint32_t prng_next_u32(prng_ctx *ctx);
uint64_t prng_next_u64(prng_ctx *ctx);
#endif
