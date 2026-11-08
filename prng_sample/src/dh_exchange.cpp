#include "dh_exchange.h"

bool perform_dh_exchange(prng_ctx *ctx, const uint512_t &p, const uint512_t &g,
                         dh_exchange_result &out, size_t max_attempts,
                         private_key_source draw)
{
    // Không để bên gọi dùng lại kết quả thành công cũ nếu lần này thất bại.
    out = dh_exchange_result();
    if (ctx == NULL || draw == NULL || p < uint512_t(5) ||
        g < uint512_t(2) || g > p - uint512_t(2))
        return false;

    const uint512_t two(2);
    const uint512_t upper = p - two;
    for (size_t attempt = 0; attempt < max_attempts; ++attempt)
    {
        const uint512_t a = draw(ctx, p);
        const uint512_t b = draw(ctx, p);
        // Khóa ngoài khoảng là lỗi nguồn sinh; retry sẽ che lỗi hợp đồng.
        // Hai khóa bằng nhau vẫn hợp lệ, không bắt buộc a != b.
        if (a < two || a > upper || b < two || b > upper)
            return false; // Broken source contract, not an unlucky draw.

        dh_exchange_result candidate;
        candidate.alice_public = modular_exponentiation(g, a, p);
        candidate.bob_public = modular_exponentiation(g, b, p);
        // Với safe prime và g sinh toàn nhóm, a=q hoặc b=q tạo public p-1.
        // Loại public 0, 1, p-1 và sinh lại cả hai khóa; không báo lỗi ngay.
        if (candidate.alice_public < two || candidate.alice_public > upper ||
            candidate.bob_public < two || candidate.bob_public > upper)
            continue; // Includes a=q or b=q, whose public key is p-1.

        candidate.alice_secret = modular_exponentiation(candidate.bob_public, a, p);
        candidate.bob_secret = modular_exponentiation(candidate.alice_public, b, p);
        // Theo DH hai kết quả phải trùng; khác nhau phải dừng để phát hiện lỗi.
        if (candidate.alice_secret != candidate.bob_secret)
            return false; // Arithmetic error must not be hidden by retries.
        // Bằng nhau chưa đủ: bí mật 0, 1 hoặc p-1 bị demo coi là suy biến.
        // Với safe prime/g đúng và public đã lọc, đây còn là bước bảo vệ bổ sung.
        if (candidate.alice_secret < two || candidate.alice_secret > upper)
            continue;
        candidate.attempts = attempt + 1;
        out = candidate;
        return true;
    }
    // Hết ngân sách thử: tránh vòng lặp vô hạn khi nguồn khóa liên tục suy biến.
    return false;
}
