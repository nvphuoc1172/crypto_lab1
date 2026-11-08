#ifndef DH_EXCHANGE_H
#define DH_EXCHANGE_H

#include "dh_sample.h"

// Kết quả trao đổi thành công; không lưu khóa riêng của Alice/Bob.
// attempts đếm số cặp đã thử (gồm cặp bị loại), bằng 0 khi thất bại.
struct dh_exchange_result
{
    uint512_t alice_public;
    uint512_t bob_public;
    uint512_t alice_secret;
    uint512_t bob_secret;
    size_t attempts = 0;
};

// Mặc định dùng PRNG thật; test truyền nguồn khóa xác định để ép ca biên.
typedef uint512_t (*private_key_source)(prng_ctx *, const uint512_t &);

// Bên gọi phải seed ctx, xác minh safe prime p=2q+1 và phần tử sinh g.
// Hàm chỉ kiểm tra khoảng, không tự chứng minh nguyên tố hay kiểm tra bậc g.
// Sinh lại cả cặp khi public key hoặc shared secret ngoài [2,p-2].
// Ví dụ a=q hợp lệ với hàm sinh khóa nhưng g^q=p-1 nên bị demo loại.
// NULL, nguồn khóa sai khoảng, hai bí mật khác nhau hoặc hết lượt -> false.
// Tối đa max_attempts cặp (mặc định 128); out được xóa khi thất bại.
// Không đổi hợp đồng generate_private_key: khóa vẫn được lấy trong [2,p-2].
bool perform_dh_exchange(prng_ctx *ctx, const uint512_t &p, const uint512_t &g,
                         dh_exchange_result &out, size_t max_attempts = 128,
                         private_key_source draw = generate_private_key);

#endif
