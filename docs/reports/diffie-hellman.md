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
g++ -std=c++14 -O3 -Wall -Wextra -Wpedantic -Wvla -Wredundant-decls src/main_dh.cpp src/private_key.cpp src/prime_sample.cpp src/dh_sample.cpp src/u512_sample.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I src -I lib/fips202 -ladvapi32 -o bin/dh.exe
if ($LASTEXITCODE -eq 0) { & .\bin\dh.exe --test }
```

Sau khi build thành công, chạy chế độ mặc định bằng:

```powershell
& .\bin\dh.exe
```

Sinh safe prime 512 bit có thể mất thời gian; chương trình in thông báo
trước khi bắt đầu. Chưa có số đo thời gian thực thi được xác nhận.

## 5. Kết quả kiểm thử và lỗi ghi nhận

### Kết quả thực tế

| Kiểm tra | Kết quả ghi nhận |
| --- | --- |
| Build `main_dh.cpp` và các mô-đun phụ thuộc bằng MinGW GCC 6.3.0, C++14 | Thành công, exit code 0, không có cảnh báo với các cờ nêu trên |
| Chạy `bin/dh.exe --test` trong sandbox | Windows Application Control chặn executable |
| Chạy lại `bin/dh.exe --test` ngoài sandbox | Vẫn bị cùng chính sách chặn |
| Trao đổi khóa mặc định với safe prime 512 bit | Chưa chạy xác nhận |

Thông báo lỗi quan sát được:

```text
An Application Control policy has blocked this file
```

Đây là lỗi môi trường thực thi. Chưa có kết quả PASS/FAIL của luồng trao
đổi khóa và chưa ghi nhận được lỗi thuật toán qua chạy chương trình.
Build thành công không đủ để kết luận trao đổi khóa đã chạy đúng.

Khi chạy được, kết quả thành công mong đợi kết thúc bằng:

```text
Diffie-Hellman succeeded: shared secrets match.
```

### Các lỗi chương trình có xử lý

Các nhánh dưới đây đã có trong mã nguồn, nhưng chưa được xác nhận bằng
thực thi do executable bị chặn:

| Điều kiện | Thông báo / hành vi |
| --- | --- |
| Đối số không hợp lệ | In `Usage: ... [--test]`, trả về 1 |
| Không lấy được entropy hệ điều hành | `Cannot obtain system randomness.`, trả về 1 |
| Tham số không đạt kiểm tra nguyên tố hoặc độ dài | `Invalid safe prime parameters.`, trả về 1 |
| Không tìm được phần tử sinh | `Cannot find a generator.`, trả về 1 |
| Khóa riêng ngoài khoảng | `Invalid private key.`, trả về 1 |
| Khóa công khai ngoài khoảng | `Invalid public key.`, trả về 1 |
| Hai bí mật khác nhau | `Diffie-Hellman failed: shared secrets differ.`, trả về 1 |

### Giới hạn hiện tại

- Đây là mô phỏng bài tập trong một tiến trình, chưa có truyền mạng hay
  xác thực đối tác.
- Bí mật chung là kết quả số học; chưa có bước dẫn xuất khóa mã hóa (KDF).
- Khóa riêng có thể bằng `q`, khi đó khóa công khai bằng `p-1` và bị
  kiểm tra khoảng từ chối. Chương trình hiện dừng với lỗi, chưa tự sinh lại
  khóa trong trường hợp này. Với `p=23`, đây là trường hợp cần chú ý khi
  mở rộng kiểm thử sang các seed khác.
- Kiểm tra bằng nhau không bảo đảm bí mật chung luôn khác 1; chương trình
  hiện chưa kiểm tra riêng trường hợp bí mật suy biến này.
- Chưa xác nhận thời gian chạy, trao đổi khóa 512 bit và các nhánh lỗi
  bằng thực thi trên môi trường cho phép chạy executable.
