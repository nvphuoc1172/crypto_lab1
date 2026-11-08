#include "prng.h"
#include <stdlib.h>
#include <string.h>

// Windows CryptoAPI / Linux getrandom. Never fall back to rand().
void randombytes(uint8_t *out, uint8_t outlen)
{
    if (outlen == 0)
        return;
    if (out == NULL)
        abort();
#if defined(_WIN32)
    HCRYPTPROV h_prov = 0;
    if (CryptAcquireContext(&h_prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        const BOOL ok = CryptGenRandom(h_prov, outlen, out);
        CryptReleaseContext(h_prov, 0);
        if (!ok)
            abort();
    }
    else
    {
        abort();
    }
#else
    ssize_t ret;
    while (outlen > 0)
    {
        ret = getrandom(out, outlen, 0);
        if ((ret == -1) && (errno == EINTR))
        {
            continue;
        }
        else if (ret <= 0)
        {
            abort();
        }
        out += ret;
        outlen -= ret;
    }
#endif
}

void prng_seed(prng_ctx *ctx, const uint8_t *seed, size_t seedlen)
{
    shake128_init(&ctx->sponge);
    shake128_absorb(&ctx->sponge, seed, seedlen);
    shake128_finalize(&ctx->sponge);
}

uint16_t prng_next_u16(prng_ctx *ctx)
{
    uint8_t buf[2];
    shake128_squeeze(buf, sizeof(buf), &ctx->sponge);
    return (uint16_t)buf[0] | (uint16_t)(buf[1] << 8);
}

uint32_t prng_next_u32(prng_ctx *ctx)
{
    uint8_t buf[4];
    shake128_squeeze(buf, sizeof(buf), &ctx->sponge);
    return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}

uint64_t prng_next_u64(prng_ctx *ctx)
{
    uint8_t buf[8];
    shake128_squeeze(buf, sizeof(buf), &ctx->sponge);
    uint64_t ret = 0;
    for (size_t i = 0; i < 8; i++)
    {
        ret |= (uint64_t)buf[i] << (8 * i);
    }
    return ret;
}
