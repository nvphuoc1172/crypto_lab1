# CryptoLab - Lab01

C++14: 512/1024-bit arithmetic, prime generation and Diffie-Hellman.

Hướng dẫn chung cho Windows (PowerShell, CMD, Git Bash) và Linux/WSL:
[build, kiểm tra và chạy chương trình](build_test_guide.md).

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File prng_sample/build.ps1 -Test -TestSafe512
./prng_sample/bin/dh.exe
```

Tài liệu các phần chính: [nền tảng số lớn](docs/reports/big_integer.md),
[sinh và kiểm tra số nguyên tố](docs/reports/prime_generation.md),
[trao đổi Diffie–Hellman](docs/reports/diffie-hellman.md).

## Module dh_exchange

`main_dh.cpp` seed PRNG, sinh/kiểm tra safe prime `p=2q+1` và chọn phần tử sinh
`g`, rồi gọi `perform_dh_exchange`. Đây là module nội bộ, dùng lại
`generate_private_key` và `modular_exponentiation`, không phải thư viện bên ngoài.

| File | Vai trò |
| --- | --- |
| `src/dh_exchange.h` | Khai báo hàm và kết quả trao đổi; nêu điều kiện đầu vào |
| `src/dh_exchange.cpp` | Sinh khóa, tính public/shared secret và xử lý cặp suy biến |
| `src/test_dh_exchange.cpp` | Truyền nguồn khóa giả lập để ép các trường hợp biên |

Các đường dẫn trong bảng tính từ `prng_sample/`. Cách xử lý:

- Khóa riêng bằng `q` vẫn thuộc `[2,p−2]`, nhưng public key bằng `p−1`: loại cả cặp và sinh lại.
- Public key hoặc bí mật chung ngoài `[2,p−2]` (gồm `0`, `1`, `p−1`): sinh lại cặp.
- Hai bí mật khác nhau hoặc nguồn sinh trả khóa ngoài khoảng: thất bại ngay, tránh che lỗi.
- Tối đa 128 cặp; hết lượt trả thất bại và xóa kết quả đầu ra. Không ép hai khóa phải khác nhau.

Test ép cặp chứa `q`, hết lượt thử, đầu vào lỗi và bí mật bằng `1`. Ca bí mật
bằng `1` dùng nhóm không-safe-prime chỉ để kiểm tra nhánh bảo vệ; demo thật
luôn xác minh safe prime trước khi trao đổi.

Test này tự chạy với `build.ps1 -Test` hoặc `make test`. Chạy riêng trên Windows:

```powershell
cd prng_sample
mingw32-make test-dh-exchange
.\bin\test-dh-exchange.exe
```

Linux/WSL dùng `make test-dh-exchange` rồi `./bin/test-dh-exchange`.
`dh.exe --test` chạy demo với p=23 và in hai bí mật; `dh.exe` mặc định dùng
512 bit và không in khóa riêng/bí mật. Chưa hoàn thiện báo cáo PDF và ZIP bài nộp.
