# Hàm generate_private_key

## 1. Hàm thực hiện, đầu vào và đầu ra

Hàm sinh khóa riêng cho mỗi bên tham gia trao đổi khóa Diffie–Hellman.
Khai báo trong `prng_sample/src/dh_sample.h`, cài đặt trong
`prng_sample/src/private_key.cpp`:

```cpp
uint512_t generate_private_key(prng_ctx *ctx, const uint512_t &p);
```

| Tham số | Ý nghĩa | Điều kiện |
| --- | --- | --- |
| `ctx` | Con trỏ đến trạng thái bộ sinh số giả ngẫu nhiên SHAKE128 | Khác `NULL`, đã khởi tạo bằng `prng_seed` |
| `p` | Modulus nguyên tố công khai đã được Alice và Bob thống nhất | `p >= 5`, biểu diễn được bằng `uint512_t` |

Đầu ra là khóa riêng kiểu `uint512_t`, thuộc đoạn **`[2, p-2]`**.
Hàm trả về `0` nếu `ctx == NULL` hoặc `p < 5`; giá trị này không phải khóa hợp lệ.
Hàm làm thay đổi trạng thái PRNG khi lấy số ngẫu nhiên, không sửa `p`.

Hàm không tự kiểm tra tính nguyên tố hay điều kiện safe prime của `p`.
Bên gọi cần cung cấp `p` phù hợp yêu cầu Diffie–Hellman, trong đó cả `p`
và `(p-1)/2` đều là số nguyên tố theo yêu cầu bài tập.

Ví dụ sau khi `ctx` đã được seed và `p` đã hợp lệ:

```cpp
const uint512_t alice_private = generate_private_key(&ctx, p);
const uint512_t bob_private = generate_private_key(&ctx, p);
```

Hai lần gọi lấy các giá trị tiếp theo từ PRNG. Hai khóa có thể trùng nhau
một cách ngẫu nhiên; hàm không ép chúng phải khác nhau. Seed cố định chỉ
dùng cho học tập và kiểm thử, không dùng để bảo vệ khóa thực tế.

## 2. Cơ chế hoạt động

Hàm sử dụng **rejection sampling** (lấy mẫu và loại bỏ giá trị ngoài khoảng):

1. Kiểm tra đầu vào; trả về `0` nếu con trỏ rỗng hoặc `p < 5`.
2. Tính `range = p - 3`, là số giá trị trong đoạn `[2, p-2]`.
3. Tính `bits = bit_length(range - 1)`, là số bit cần để biểu diễn giá trị
   lớn nhất của đoạn `[0, range-1]`.
4. Khởi tạo ứng viên bằng `0`, điền các limb 64 bit bằng `prng_next_u64`.
   Chỉ lấy đủ số limb cần thiết và xóa các bit thừa ở limb cao nhất.
5. Nếu `candidate >= range`, bỏ ứng viên và sinh lại.
6. Trả về `candidate + 2`, chuyển đoạn `[0, p-4]` thành `[2, p-2]`.

Ví dụ với `p = 23`: `range = 20`, cần 5 bit. Ứng viên nằm trong `[0, 31]`;
các giá trị từ 20 đến 31 bị loại, các giá trị từ 0 đến 19 được cộng 2
để tạo khóa từ 2 đến 21.

Với giả thiết các bit đầu ra PRNG phân bố đều, mỗi khóa hợp lệ có cùng
xác suất được chọn. Lấy phần dư trực tiếp từ một khoảng ngẫu nhiên có
kích thước không chia hết cho `range` có thể gây lệch phân bố, nên hàm
dùng bước loại bỏ. Xác suất chấp nhận mỗi lượt lớn hơn 1/2, vì vậy số
lượt sinh trung bình nhỏ hơn 2; không có giới hạn cố định cho số lượt lặp.

## 3. Kiểm thử và lỗi ghi nhận

Bộ test: `prng_sample/src/test_private_key.cpp`.
Seed test là mảng 32 byte khởi tạo đầy đủ: `const uint8_t seed[32] = {42};`.

| Nhóm kiểm tra đã viết | Kết quả mong đợi |
| --- | --- |
| `ctx == NULL` | Trả về `0` |
| `p` từ 0 đến 4 | Trả về `0` |
| `p = 5`, lấy 128 mẫu | Chỉ sinh 2 hoặc 3, quan sát được cả hai đầu mút |
| Hai PRNG cùng seed | Cho cùng chuỗi khóa với cùng chuỗi lời gọi |
| Khoảng có độ rộng 5, 64, 65, 128, 511 và 512 bit, mỗi khoảng 128 mẫu | Mọi khóa thuộc `[2, p-2]` |
| Alice/Bob với `p = 23`, `g = 5` | Hai bên tính được cùng bí mật chung |

Các modulus trong test độ rộng là `2^bits - 1`, dùng để kiểm tra thao tác
bit và giới hạn khoảng; chúng không được giả định đều là số nguyên tố.
Kiểm tra bí mật chung hiện dùng modulus nhỏ, chưa xác nhận trao đổi khóa
với safe prime 512 bit. Các mẫu hữu hạn cũng không chứng minh phân bố đều;
tính chất đó dựa trên cơ chế lấy mẫu và giả thiết về PRNG.

### Runtime status

Windows/MinGW GCC 6.3.0: both private-key test suites and full 512-bit DH passed on 2026-11-08. See [current status](CURRENT_STATUS.md). Earlier Application Control failures were environment-specific historical runs.

### Cách chạy lại

Từ thư mục `prng_sample/`, trên Linux/WSL:

```sh
make run-test-private-key
```

Trên PowerShell với MinGW (PowerShell):

```powershell
g++ -std=c++14 -O3 -Wall -Wextra -Wpedantic -Wvla -Wredundant-decls src/test_private_key.cpp src/private_key.cpp src/dh_sample.cpp src/u512_sample.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I src -I lib/fips202 -ladvapi32 -o bin/test-private-key.exe
if ($LASTEXITCODE -eq 0) { & .\bin\test-private-key.exe }
```

Không thêm `-DNDEBUG`, vì bộ test sử dụng `assert`.
Khi chạy thành công, chương trình dự kiến in:

```text
Private key tests passed (including Alice/Bob shared secret).
```
