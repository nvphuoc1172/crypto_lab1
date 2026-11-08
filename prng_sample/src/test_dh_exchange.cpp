#include "dh_exchange.h"
#include <cassert>
#include <iostream>

namespace
{
// Nguồn giả lập chỉ dành cho test: trả đúng chuỗi khóa đã chỉ định.
// Không cần chờ PRNG tình cờ sinh q hoặc một cặp tạo bí mật bằng 1.
const uint64_t *sequence;
size_t length;
size_t position;
uint512_t scripted_key(prng_ctx *, const uint512_t &)
{
    assert(position < length);
    return uint512_t(sequence[position++]);
}
void use(const uint64_t *values, size_t count)
{
    sequence = values;
    length = count;
    position = 0;
}
}

int main()
{
    prng_ctx ctx;
    const uint8_t seed[32] = {42};
    prng_seed(&ctx, seed, sizeof(seed));
    dh_exchange_result result;
    // p=23, q=11, g=5: cặp (11,2) tạo public 22=p-1, phải bị loại.
    // Cặp tiếp theo (2,3) hợp lệ, cho bí mật 8; hoàn tất ở lượt thứ 2.
    const uint64_t retry_q[] = {11, 2, 2, 3};
    use(retry_q, 4);
    assert(perform_dh_exchange(&ctx, uint512_t(23), uint512_t(5), result, 2, scripted_key));
    assert(result.attempts == 2 && position == 4);
    assert(result.alice_secret == result.bob_secret && result.alice_secret == uint512_t(8));

    // q xuất hiện lần lượt ở Alice/Bob; cả hai lượt đều bị loại.
    // Hàm trả false và xóa kết quả cũ, thay vì lặp mãi hay trả bí mật cũ.
    const uint64_t always_q[] = {11, 2, 2, 11};
    use(always_q, 4);
    assert(!perform_dh_exchange(&ctx, uint512_t(23), uint512_t(5), result, 2, scripted_key));
    assert(result.attempts == 0 && result.alice_secret.is_zero());

    // Synthetic non-safe-prime group exercises the shared-secret=1 guard:
    // p=31, g=3, a=3, b=10 -> public keys 27/25, shared secret 1.
    // q=15 không nguyên tố: cố ý dùng nhóm này chỉ để chạm nhánh bảo vệ.
    // Demo thật luôn xác minh safe prime trước, không dùng p=31 làm tham số.
    // Cặp tiếp theo (2,3) cho bí mật 16 và được chấp nhận.
    const uint64_t retry_one[] = {3, 10, 2, 3};
    use(retry_one, 4);
    assert(perform_dh_exchange(&ctx, uint512_t(31), uint512_t(3), result, 2, scripted_key));
    assert(result.attempts == 2 && result.alice_secret == uint512_t(16));

    // Đầu vào NULL, ngân sách 0 và nguồn sinh khóa 0 đều phải thất bại.
    assert(!perform_dh_exchange(NULL, uint512_t(23), uint512_t(5), result));
    assert(!perform_dh_exchange(&ctx, uint512_t(23), uint512_t(5), result, 0));
    const uint64_t invalid[] = {0, 2};
    use(invalid, 2);
    assert(!perform_dh_exchange(&ctx, uint512_t(23), uint512_t(5), result, 2, scripted_key));
    std::cout << "DH exchange edge tests passed.\n";
}
