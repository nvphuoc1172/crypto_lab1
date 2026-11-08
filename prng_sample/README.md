## Lab01: Số nguyên tố & Trao đổi khóa Diffie-Hellman

### Chạy chương trình trao đổi khóa hoàn chỉnh

`src/main_dh.cpp` mô phỏng Alice và Bob trong một tiến trình: sinh safe prime
512 bit `p=2q+1`, kiểm tra lại `p` và `q`, tìm phần tử sinh `g`, sinh khóa
riêng trong `[2,p-2]`, trao đổi khóa công khai và so sánh hai bí mật chung.
Với safe prime, `g` được chọn sao cho `g^2 mod p != 1` và `g^q mod p != 1`.
Chế độ mặc định seed PRNG từ hệ điều hành, dừng nếu lấy entropy thất bại;
chỉ in tham số và khóa công khai. Sinh safe prime 512 bit có thể mất thời gian.

Từ `prng_sample/` trên Linux/WSL:

```sh
make run-dh       # Sinh tham số 512 bit và trao đổi khóa
make test-dh      # Kiểm tra nhanh với p=23 và seed cố định
```

Trên PowerShell với MinGW:

```powershell
g++ -std=c++14 -O3 -Wall -Wextra -Wpedantic src/main_dh.cpp src/private_key.cpp src/prime_sample.cpp src/dh_sample.cpp src/u512_sample.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I src -I lib/fips202 -ladvapi32 -o bin/dh.exe
if ($LASTEXITCODE -eq 0) { & .\bin\dh.exe --test }
# Sau khi build thành công, chạy chế độ 512 bit:
# & .\bin\dh.exe
```

Chế độ `--test` dùng seed cố định đã khởi tạo đầy đủ và in hai bí mật để
đối chiếu; không dùng các giá trị này để bảo vệ dữ liệu. Chương trình trả
về 0 khi hai bí mật khớp, 1 khi có lỗi. Đây là mô phỏng bài tập, chưa có
truyền mạng, xác thực đối tác hoặc bước dẫn xuất khóa mã hóa từ bí mật chung.

### Sinh khóa riêng Alice và Bob

Sau khi seed `prng_ctx` và thống nhất số nguyên tố `p`, gọi hai lần:

```cpp
const uint512_t alice_private = generate_private_key(&ctx, p);
const uint512_t bob_private = generate_private_key(&ctx, p);
```

Hàm trong `src/private_key.cpp` dùng rejection sampling để sinh đều trong
`[2, p-2]`, hỗ trợ đến 512 bit. Trả về 0 nếu `ctx == NULL` hoặc `p < 5`;
bên gọi chịu trách nhiệm kiểm tra `p` là số nguyên tố và khởi tạo PRNG.
Hai khóa được lấy độc lập, có thể trùng nhau một cách ngẫu nhiên.

Chạy test trên Linux/WSL: `make run-test-private-key`.
Trên PowerShell, từ `prng_sample/`:

```powershell
g++ -std=c++14 -O3 -Wall -Wextra -Wpedantic src/test_private_key.cpp src/private_key.cpp src/dh_sample.cpp src/u512_sample.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I src -I lib/fips202 -ladvapi32 -o bin/test-private-key.exe
if ($LASTEXITCODE -eq 0) { & .\bin\test-private-key.exe }
```

Test dùng seed cố định đã khởi tạo đầy đủ, kiểm tra giới hạn khóa, tái lập
kết quả, các ranh giới limb đến 512 bit và bí mật chung Alice/Bob với `p=23`.
Seed cố định chỉ dành cho test; khi tích hợp cần liên kết thêm `private_key.cpp`.

Repo cung cấp thư viện sinh số giả ngẫu nhiên (PRNG) dùng SHAKE128, một XOF
(Extendable-Output Function).

```
make run-prng
```

Dùng các hàm trong thư viện để tự xây dựng số lớn, 512 bit, 1024 bit, etc. (uint512_t, uint1024_t uint2048_t).

### Sinh và kiểm tra số nguyên tố

`src/prime_sample.cpp` cung cấp:

- `is_probable_prime(n, &ctx, rounds)`: lọc chia thử bằng các số nguyên tố nhỏ,
  sau đó kiểm tra Miller–Rabin. Mặc định 40 vòng; với cơ sở ngẫu nhiên độc lập,
  xác suất chấp nhận nhầm một hợp số không quá `4^-40`.
- `generate_prime(&ctx, bitsize, rounds)`: sinh số nguyên tố probable-prime có
  đúng số bit yêu cầu (2 đến 512).
- `generate_safe_prime(&ctx, bitsize)`: tìm `p = 2q + 1`, kiểm tra cả `q` và
  `p` bằng Miller–Rabin 40 vòng; hỗ trợ 3 đến 512 bit.

Khởi tạo `prng_ctx` bằng `prng_seed` trước khi gọi các hàm sinh. Với kiểm thử,
dùng seed cố định; seed đó không phù hợp để bảo vệ khóa thật. Giá trị trả về 0
biểu thị tham số đầu vào không hợp lệ. Mô-đun có thể biên dịch độc lập bằng
`make prime-module`; khi tích hợp chương trình, liên kết thêm `prime_sample.cpp`,
`dh_sample.cpp`, `u512_sample.cpp`, `prng.cpp` và `fips202.c`.

Nhân modulo cho modulus lẻ dùng Montgomery reduction để tăng tốc Miller–Rabin.
Đường nhân 512x512 cũng có bản schoolbook limb 32 bit cho compiler không hỗ trợ
`__int128`.
### Windows (PowerShell + MinGW g++)

Chạy từ thư mục `prng_sample`:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1 -Test
```

Script biên dịch bằng C++14 rồi chạy demo hoặc bộ kiểm tra PRNG. `Bypass` chỉ
áp dụng cho tiến trình này, không thay đổi execution policy toàn hệ thống.
Không cần WSL hay thư viện mật mã cài thêm: `randombytes` gọi BCryptGenRandom
từ DLL trong thư mục hệ thống Windows; lỗi lấy entropy sẽ dừng chương trình.
Nhánh Linux vẫn dùng `getrandom`. Makefile hiện dành cho môi trường Unix.

Demo sử dụng seed cố định, khởi tạo đầy đủ để tái hiện kết quả. Để lấy seed
từ hệ điều hành, dùng `randombytes(seed, sizeof(seed))` với seed 32 byte.
API `randombytes` hiện nhận độ dài `uint8_t`, tối đa 255 byte mỗi lần gọi.

Bộ kiểm tra xác nhận yêu cầu độ dài 0, giới hạn ghi buffer và thứ tự byte
của các hàm PRNG so với luồng SHAKE128; không đánh giá chất lượng thống kê RNG.
Trên Linux có thể chạy bộ kiểm tra bằng:

```sh
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic tests/prng_test.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I lib/fips202 -o bin/prng_test
./bin/prng_test
```

Phần số lớn trong `u512_sample.h` vẫn có `__uint128_t`, là phần mở rộng
compiler; thay đổi PRNG này chưa chuyển phần đó sang MSVC/MinGW 32 bit.
