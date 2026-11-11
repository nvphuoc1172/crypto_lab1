# Nền tảng số nguyên lớn

Tài liệu này mô tả phần biểu diễn và toán tử số lớn dùng chung cho Lab01,
trọng tâm là `uint512_t` theo phân công A. Các phép nhân và lũy thừa mô-đun
được trình bày ở tài liệu của module tương ứng.

## Biểu diễn

`uint512_t` lưu số nguyên **không dấu 512 bit** trong tám limb `uint64_t`.
Mảng `digits` dùng thứ tự little-endian: `digits[0]` giữ 64 bit thấp nhất,
`digits[7]` giữ 64 bit cao nhất. Giá trị biểu diễn là

`digits[0] + digits[1]·2^64 + … + digits[7]·2^448`.

Đây là số có độ rộng cố định, không phải số nguyên tùy ý. `uint1024_t` gồm 16
limb theo cùng quy ước và dùng làm vùng trung gian để giữ đầy đủ tích của hai
số 512 bit trước khi lấy modulo.

## API và hành vi

| API | Hành vi |
| --- | --- |
| `uint512_t()` / `uint512_t(uint64_t)` | Khởi tạo số 0 hoặc mở rộng một số 64 bit không dấu. |
| `bit(i)` / `set_bit(i, value)` | Đọc hoặc đặt bit theo chỉ số từ 0 (bit thấp nhất) đến 511. Ngoài miền, `bit` trả `false`, còn `set_bit` không làm gì. |
| `bit_length()` | Số bit cần để biểu diễn giá trị; trả 0 với số 0. |
| `is_zero`, `is_one`, `is_odd`, `is_even` | Các phép kiểm tra giá trị cơ bản. |
| `+`, `-`, `+=`, `-=` | Cộng và trừ không dấu theo modulo `2^512`; carry cuối hoặc borrow cuối bị bỏ. Toán tử không báo tràn. |
| `<<`, `>>`, `<<=`, `>>=` | Dịch logic; bit vượt khỏi 512 bit bị bỏ. Dịch từ 512 bit trở lên trả 0. |
| `==`, `!=`, `<`, `>`, `<=`, `>=` | So sánh giá trị không dấu, bắt đầu từ limb cao nhất. |
| `to_hex`, `operator<<` | Xuất hex chữ thường có tiền tố `0x`; không đệm số 0 ở đầu, số 0 xuất thành `0x0`. |
| `from_hex` | Đọc 1–128 chữ số hex, có thể có tiền tố `0x`/`0X`; ném `std::invalid_argument` nếu không có chữ số, sai ký tự hoặc có hơn 128 chữ số. |

Ví dụ: `uint512_t(1) << 64` có `digits[0] = 0`, `digits[1] = 1`. Phép
`uint512_t() - uint512_t(1)` tạo giá trị lớn nhất `2^512 - 1` theo quy tắc
wrap của số không dấu cố định độ rộng.

`uint1024_t` cung cấp các thao tác bit, cộng/trừ/dịch và so sánh cùng quy ước
little-endian ở độ rộng 1024 bit. Các helper `mul`, `mod`, `mul_mod`, `add_mod`
và `sub_mod` được khai báo tại `prng_sample/src/u512_sample.h`; hợp đồng phép
toán mô-đun và tối ưu Montgomery thuộc phần B, không phải toán tử cơ bản của A.

## Kiểm thử phần A

Các ca trong `prng_sample/src/test_u512.cpp` kiểm tra:

- Khởi tạo 0/1, cộng/trừ, carry và borrow qua nhiều limb, wrap tại biên `2^512`.
- So sánh hai số khác nhau ở limb thấp và limb cao.
- Dịch qua ranh giới limb, bit 63/64/511 và dịch đúng độ rộng 512 bit.
- Đọc, đặt, xóa bit; chỉ số bit ngoài miền.
- Chuyển đổi hex hai chiều và xuất qua `operator<<`, bao gồm giá trị 512 bit lớn nhất.
- Tích 64×64 được giữ trong 128 bit và các ca modulo/lũy thừa mô-đun cơ bản.

Một số regression chung ở `prng_sample/src/test_regressions.cpp` kiểm tra parser
hex từ chối chuỗi sai và phép modulo 512 bit với modulus 64 bit lớn.

Chạy toàn bộ build và test trên Windows/MinGW từ thư mục gốc:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\prng_sample\build.ps1 -Test
```

Script biên dịch với `-std=c++14`, cảnh báo bật và không định nghĩa `NDEBUG`,
nên các `assert` trong test vẫn hoạt động. Theo trạng thái đã ghi trong
`CURRENT_STATUS.md`, bộ test số lớn và regression đã chạy qua trên Windows/
MinGW GCC 6.3.0. Ngày 11/11/2026, sau khi bổ sung các ca biên của phần A,
lệnh trên build và chạy thành công tất cả test trong script. Linux/WSL và MSVC
chưa được xác nhận.

## Giới hạn

- Phép cộng/trừ wrap theo `2^512`, không trả cờ carry/borrow. Caller cần tự
  kiểm tra biên nếu không muốn kết quả wrap.
- Parser chỉ nhận hex không dấu; không nhận dấu, dấu cách hoặc dấu phân cách.
- `digits` là trường công khai để các module số học truy cập trực tiếp; caller
  có thể sửa limb thủ công và phải tự bảo đảm giá trị mong muốn.
- `uint512_t` không tự bảo vệ dữ liệu bí mật khỏi bị quan sát qua thời gian
  thực thi hay xóa khỏi bộ nhớ. Phần này là nền tảng học tập, không phải thư viện
  mật mã đã được đánh giá an toàn.
