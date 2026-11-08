# Báo cáo hoàn thành trao đổi khóa Diffie–Hellman

## 1. Nội dung đã chỉnh sửa

- Thêm `prng_sample/src/main_dh.cpp`: mô phỏng toàn bộ quá trình trao đổi
  khóa giữa Alice và Bob trong cùng một tiến trình.
- Thêm các target `dh`, `run-dh`, `test-dh` trong `prng_sample/Makefile`.
- Bổ sung hướng dẫn build và chạy trong `prng_sample/README.md`.
- Tích hợp các hàm đã có: sinh safe prime, kiểm tra nguyên tố,
  sinh khóa riêng và lũy thừa modulo.

## 2. Hàm thực hiện, đầu vào và đầu ra

### Hàm main

```cpp
int main(int argc, char **argv);
```

| Đầu vào | Ý nghĩa |
| --- | --- |
| Không có đối số | Sinh safe prime 512 bit, khởi tạo PRNG từ nguồn ngẫu nhiên hệ điều hành |
| `--test` | Kiểm tra nhanh với `p = 23`, seed cố định 32 byte `{42, 0, ..., 0}` |
| Đối số khác hoặc thừa đối số | In cách sử dụng và trả về mã lỗi 1 |

Chương trình không yêu cầu người dùng nhập khóa riêng hay tham số bằng bàn phím.
Phần tạo khóa/tính bí mật hiện được gọi qua `perform_dh_exchange` trong
`dh_exchange.cpp`. Nó thử lại cặp suy biến tối đa 128 lần; mã ở các bước dưới
minh họa công thức, không phải toàn bộ vòng xử lý thực tế.
Trong chế độ mặc định, kích thước modulus được cố định là 512 bit.

| Đầu ra | Nội dung |
| --- | --- |
| `stdout` | Thông báo tiến trình, `p`, `q`, `g`, khóa công khai của Alice/Bob và thông báo thành công |
| `stdout` khi `--test` | In thêm hai bí mật chung để đối chiếu |
| `stderr` | Thông báo lỗi khi một bước thất bại |
| Mã trả về `0` | Các bước kiểm tra đạt yêu cầu và hai bí mật chung bằng nhau |
| Mã trả về `1` | Tham số dòng lệnh không hợp lệ hoặc một bước xử lý thất bại |

Chế độ mặc định không in khóa riêng và bí mật chung.
Các số lớn được xuất theo định dạng hex của `uint512_t`.

### Hàm seed_from_system

```cpp
bool seed_from_system(prng_ctx &ctx);
```

Đây là hàm nội bộ trong `main_dh.cpp`. Đầu vào là tham chiếu đến trạng thái
PRNG cần khởi tạo; đầu ra là `true` nếu lấy được đủ 32 byte seed và đã gọi
`prng_seed`, hoặc `false` nếu lấy entropy thất bại.

- Windows: dùng `CryptAcquireContext`, `CryptGenRandom`, sau đó giải phóng
  provider bằng `CryptReleaseContext`.
- Linux: dùng `getrandom`, đọc tiếp nếu chưa đủ byte và thử lại khi gặp `EINTR`.
- Không chuyển sang `rand()` khi nguồn ngẫu nhiên hệ điều hành thất bại.

## 3. Cơ chế hoạt động

### Bước 1: Khởi tạo PRNG

Chế độ mặc định lấy seed từ hệ điều hành. Chế độ `--test` sử dụng seed
cố định được khởi tạo đầy đủ để tái lập kết quả. Seed cố định chỉ là dữ
liệu kiểm thử, không dùng cho khóa thực tế.

### Bước 2: Sinh tham số nguyên tố

Gọi `generate_safe_prime(&ctx, 512)` để tìm `p = 2q + 1` sao cho cả
`p` và `q` vượt qua kiểm tra nguyên tố Miller–Rabin. Tính lại
`q = (p - 1) / 2`, xác nhận độ dài `p` là 512 bit ở chế độ mặc định và
kiểm tra lại cả hai bằng `is_probable_prime` (mặc định 40 vòng).
Đây là kiểm tra probable prime, không phải chứng minh nguyên tố tuyệt đối.
Chế độ test dùng `p = 23`, `q = 11` và vẫn thực hiện kiểm tra nguyên tố.

### Bước 3: Tìm phần tử sinh g

Duyệt `g` từ 2 đến `p-2`, chấp nhận ứng viên thỏa mãn:

```text
g^2 mod p != 1
g^q mod p != 1
```

Nếu `p = 2q+1` thực sự là safe prime, nhóm nhân modulo `p` có cấp `2q`.
Các ước của cấp nhóm là `1, 2, q, 2q`; hai phép kiểm tra trên loại các
ứng viên có cấp `1`, `2` hoặc `q`, nên ứng viên được chọn có cấp `p-1`.

### Bước 4: Sinh khóa riêng

```cpp
const uint512_t alice_private = generate_private_key(&ctx, p);
const uint512_t bob_private = generate_private_key(&ctx, p);
```

Đặt `a = alice_private`, `b = bob_private`. Hàm sinh khóa sử dụng rejection
sampling trong `[2, p-2]`; `main` kiểm tra lại giới hạn của cả hai khóa.

### Bước 5: Tính và trao đổi khóa công khai

```text
A = g^a mod p
B = g^b mod p
```

Alice gửi `A` cho Bob, Bob gửi `B` cho Alice. Trong chương trình hiện tại,
việc trao đổi được mô phỏng bằng cách sử dụng các biến trong cùng tiến trình.
Trước khi tính bí mật chung, chương trình kiểm tra `A`, `B` thuộc `[2, p-2]`.

### Bước 6: Tính và đối chiếu bí mật chung

```text
Alice: s_A = B^a mod p
Bob:   s_B = A^b mod p
s_A = s_B = g^(ab) mod p
```

Các phép lũy thừa gọi `modular_exponentiation`. Nếu hai kết quả khác nhau,
chương trình báo lỗi; nếu bằng nhau, in thông báo thành công và trả về 0.

## 4. Build và chạy

Thực hiện từ thư mục `prng_sample/`.

Linux/WSL:

```sh
make dh          # Build chương trình
make test-dh     # Chạy kiểm tra nhanh với --test
make run-dh      # Sinh safe prime 512 bit và trao đổi khóa
```

PowerShell với MinGW:

```powershell
g++ -std=c++14 -O3 -Wall -Wextra -Wpedantic -Wvla -Wredundant-decls src/main_dh.cpp src/dh_exchange.cpp src/private_key.cpp src/prime_sample.cpp src/dh_sample.cpp src/u512_sample.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I src -I lib/fips202 -ladvapi32 -o bin/dh.exe
if ($LASTEXITCODE -eq 0) { & .\bin\dh.exe --test }
```

Sau khi build thành công, chạy chế độ mặc định bằng:

```powershell
& .\bin\dh.exe
```

Sinh safe prime 512 bit có thể mất thời gian; chương trình in thông báo
trước khi bắt đầu. Chưa có số đo thời gian thực thi được xác nhận.

## Trạng thái triển khai và kiểm thử — 08/11/2026

Tài liệu này đồng bộ trạng thái hiện tại; các log lỗi trong REPO_REVIEW là lịch sử,
không phải danh sách lỗi còn tồn tại. Đây là ghi chú kỹ thuật, không thay thế báo cáo PDF.

## Các module

| Module | Trách nhiệm |
| --- | --- |
| u512_sample.cpp | Số lớn 512/1024 bit, Montgomery, lấy dư và parse hex |
| dh_sample.cpp | Lũy thừa mô-đun |
| prime_sample.cpp | Miller–Rabin, sinh prime và safe prime |
| private_key.cpp | Cả hai overload sinh khóa riêng; p>=5, kết quả [2,p−2] |
| dh_exchange.cpp | Tạo cặp khóa, tính bí mật và xử lý giá trị suy biến |
| main_dh.cpp | Seed hệ điều hành/test, xác minh tham số, chọn g và hiển thị kết quả |

## Hành vi demo

`dh.exe --test` dùng p=23 và seed cố định, in hai bí mật để đối chiếu.
`dh.exe` sinh safe prime 512 bit, kiểm tra p/q, chọn phần tử sinh toàn nhóm,
không in khóa riêng hoặc giá trị bí mật. Hai chế độ đều kiểm tra kết quả bằng nhau.

Khóa riêng bằng q vẫn thuộc miền của hàm sinh nhưng tạo khóa công khai p−1.
Demo loại cặp này và lấy mẫu lại. Nó cũng loại bí mật chung 0, 1, p−1. Tối đa
128 cặp; hết lượt, đầu vào/nguồn khóa không hợp lệ hoặc bí mật hai bên khác nhau
thì trả thất bại. Không retry khi có sai khác số học, tránh che lỗi thuật toán.

Hàm nguồn khóa được truyền vào test để ép tình huống khó gặp. `test_dh_exchange.cpp`
kiểm tra a=q, b=q, bí mật chung 1 trong nhóm tổng hợp cho test, hết lượt thử,
con trỏ rỗng và nguồn khóa trả 0. Nhóm không-safe-prime chỉ dùng để ép nhánh bảo vệ;
main luôn kiểm tra safe prime trước khi trao đổi.

## Kiểm chứng

Đợt đối chiếu trước sửa biên đã chạy thành công trên Windows/MinGW GCC 6.3.0,
C++14: số lớn, private key chính/mở rộng, PRNG, nguyên tố, regression, safe prime
512 bit và demo DH mặc định. Các ghi nhận App Control chặn thuộc lần chạy cũ.

Lệnh kiểm tra bản sau sửa:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File prng_sample/build.ps1 -Test -TestSafe512
.\prng_sample\bin\dh.exe
```

Từ prng_sample/, có thể dùng `mingw32-make test` trên Windows hoặc `make test`
trên Linux. Bộ test biên mới được tích hợp vào cả hai cách build.
Linux/MSVC và lỗi API entropy chưa được chạy ép lỗi xác nhận.

Chưa hoàn thiện báo cáo Word/PDF và ZIP bài nộp; các artifact này để thực hiện sau.
