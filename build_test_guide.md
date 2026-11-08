# Hướng dẫn sử dụng và chạy CryptoLab — Lab01

Cập nhật theo source ngày 08/11/2026. `main_dh.cpp` là chương trình trao đổi khóa;
`main_prng.cpp` chỉ minh họa PRNG. **Bản hiện tại không có build.ps1**; dùng các
lệnh trực tiếp bên dưới, không dùng hướng dẫn script từ phiên bản trước.

## 1. Công cụ cần có

- Windows: GCC/MinGW với g++ và thư viện Windows, compiler nằm trong PATH.
- Linux/WSL: g++, GNU Make và header phát triển Linux.
- Dùng C++14; không cần GMP, NTL hay framework test ngoài.
- Không thêm `-DNDEBUG`, vì test dùng `assert`.

Kiểm tra bằng `g++ --version` trong terminal đang dùng. Không build mọi file
`src/*.cpp` chung một executable: nhiều file có hàm main riêng.

## 2. Windows — PowerShell

### Chuẩn bị

```powershell
cd 'D:\C++\CryptoLab\prng_sample'
g++ --version
New-Item -ItemType Directory -Force -Path bin | Out-Null
```

Tất cả lệnh tiếp theo chạy từ `prng_sample/`.

### Build và chạy Diffie–Hellman

```powershell
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/main_dh.cpp src/private_key.cpp src/prime_sample.cpp src/dh_sample.cpp src/u512_sample.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I src -I lib/fips202 -ladvapi32 -o bin/dh.exe
if ($LASTEXITCODE -ne 0) { throw 'Build thất bại; không chạy executable cũ.' }
.\bin\dh.exe --test
$LASTEXITCODE
```

`--test` dùng p=23, seed cố định và in hai bí mật để đối chiếu. Chạy đúng kích thước
mặc định của đồ án bằng:

```powershell
.\bin\dh.exe
$LASTEXITCODE
```

Chế độ mặc định sinh safe prime 512 bit, kiểm tra p/q, tìm phần tử sinh g, sinh
khóa riêng, tính khóa công khai và đối chiếu bí mật chung. Thời gian sinh prime
thay đổi theo seed và máy. Kết quả cần có:

```text
Diffie-Hellman succeeded: shared secrets match.
```

Exit code 0 là thành công, 1 là lỗi. Chế độ mặc định không in giá trị bí mật;
`--test` mới in. Test nhỏ không thay thế chạy 512 bit. Chương trình mô phỏng trong
cùng tiến trình, chưa có truyền mạng hay xác thực đối tác.

### Build các test

```powershell
# Số lớn và lũy thừa
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/test_u512.cpp src/u512_sample.cpp src/dh_sample.cpp -I src -I lib/fips202 -o bin/test-u512.exe
if ($LASTEXITCODE -ne 0) { throw 'Build thất bại' }
.\bin\test-u512.exe

# Khóa riêng
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/test_private_key.cpp src/private_key.cpp src/dh_sample.cpp src/u512_sample.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I src -I lib/fips202 -ladvapi32 -o bin/test-private-key.exe
if ($LASTEXITCODE -ne 0) { throw 'Build thất bại' }
.\bin\test-private-key.exe

# Nguyên tố
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/test_prime.cpp src/prime_sample.cpp src/dh_sample.cpp src/u512_sample.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I src -I lib/fips202 -ladvapi32 -o bin/test-prime.exe
if ($LASTEXITCODE -ne 0) { throw 'Build thất bại' }
.\bin\test-prime.exe
.\bin\test-prime.exe --safe512
```

Kiểm tra `$LASTEXITCODE` ngay sau từng lần chạy. Assertion thất bại khiến chương
trình dừng tại ca lỗi đầu tiên; các ca phía sau chưa được kiểm chứng.

### Demo PRNG

```powershell
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/main_prng.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I lib/fips202 -ladvapi32 -o bin/prng.exe
if ($LASTEXITCODE -eq 0) { .\bin\prng.exe }
```

Demo dùng seed cố định đã khởi tạo đầy đủ để tái hiện; không dùng seed này bảo vệ khóa thật.

## 3. Command Prompt (CMD)

```bat
cd /d "D:\C++\CryptoLab\prng_sample"
if not exist bin mkdir bin
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/main_dh.cpp src/private_key.cpp src/prime_sample.cpp src/dh_sample.cpp src/u512_sample.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I src -I lib/fips202 -ladvapi32 -o bin/dh.exe && bin\dh.exe --test
echo Exit code: %ERRORLEVEL%
```

Sau khi build thành công, chạy `bin\dh.exe` cho chế độ 512 bit.
Các lệnh g++ ở phần PowerShell cũng dùng được trên CMD. Thay phần chạy bằng
`bin\ten-chuong-trinh.exe`; dùng `&&` để chỉ chạy khi build thành công.
Gõ `echo %ERRORLEVEL%` trên dòng riêng ngay sau lệnh cần kiểm tra.

## 4. Git Bash và Linux/WSL

Git Bash cần compiler Windows trên PATH, không tự cung cấp GCC:

```sh
cd '/d/C++/CryptoLab/prng_sample'
mkdir -p bin
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/main_dh.cpp src/private_key.cpp src/prime_sample.cpp src/dh_sample.cpp src/u512_sample.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I src -I lib/fips202 -ladvapi32 -o bin/dh.exe && ./bin/dh.exe --test
echo "Exit code: $?"
```

Linux/WSL dùng compiler Linux và Makefile:

```sh
cd '/mnt/d/C++/CryptoLab/prng_sample'
make test-dh
make run-test-u512
make run-test-private-key
make run-test-prime
./bin/test-prime --safe512
make run-dh
```

Trên Linux thường, thay đường dẫn bằng vị trí checkout. `make clean` xóa toàn bộ
bin/. Makefile hiện dùng lệnh Unix, chưa phù hợp chạy trực tiếp bằng mingw32-make
với CMD. Trên Windows dùng lệnh g++ bên trên. Linux/Git Bash chưa được chạy xác
nhận trong lần viết lại tài liệu này.

## 5. C++14 và tham số

- Đề chỉ cho C++14 trở xuống: ghi rõ `-std=c++14`, không phải `std==`.
- Không dùng structured bindings, if constexpr, std::optional, std::filesystem
  từ C++17 hoặc các tính năng mới hơn trong bản nộp.
- Có thể dùng compiler mới với C++14. Chuẩn mặc định phụ thuộc compiler, không
  luôn là C++11. MinGW GCC 6.3.0 đã kiểm tra có mức mặc định C++14.
- Kiểm tra trên PowerShell: `g++ -dM -E -x c++ NUL | Select-String '__cplusplus'`.
  CMD dùng `findstr __cplusplus`; Bash dùng `/dev/null` và `grep __cplusplus`.
  Giá trị: 201103L (C++11), 201402L (C++14), 201703L (C++17), 202002L (C++20).
- __int128 là phần mở rộng compiler, không phải kiểu chuẩn C++14. Source có
  nhánh portable khi không hỗ trợ; MSVC/macOS chưa được xác nhận.
- Số bit mặc định đặt trong `generate_safe_prime(&ctx, 512)` ở main_dh.cpp;
  thư viện hỗ trợ tối đa 512 bit. Không có tùy chọn `--bits` hiện tại.
- Khóa riêng có thể ngắn hơn 512 bit, miễn thuộc [2,p−2].

## 6. Lỗi thường gặp và Windows Security

| Triệu chứng | Hướng xử lý |
| --- | --- |
| Không nhận diện g++ | Kiểm tra PATH, mở lại terminal |
| Không thấy source | Chạy từ prng_sample/ |
| Undefined reference tới generate_private_key | Liên kết src/private_key.cpp |
| Undefined reference tới Crypt... | Thêm -ladvapi32 trên Windows |
| Multiple definition of main | Build từng entry point riêng |
| Assertion failed | Ghi file/dòng lỗi, không tắt assert |
| Application Control chặn executable | Build có thể thành công, runtime chưa được xác nhận |

ExecutionPolicy Bypass, đổi terminal hay chạy Administrator không tự cho phép
executable bị App Control chặn. Kiểm tra Windows Security và log
Microsoft-Windows-CodeIntegrity/Operational; event 3077 ghi nhận policy chặn.
Máy do tổ chức quản lý cần quản trị viên cho phép ứng dụng theo chính sách.
Hướng dẫn này không đổi hoặc tắt cấu hình bảo vệ hệ thống.

## 7. Trạng thái kiểm chứng và bàn giao

Các entry point DH, private key, prime, số lớn và PRNG đã build trực tiếp với
MinGW GCC 6.3.0/C++14 trong lần đối chiếu gần nhất. Một số executable bị App
Control chặn, nên chưa kết luận bản hiện tại qua toàn bộ runtime. Không dùng
kết quả của binary cũ để xác nhận source hiện tại.

Khi sửa source phải build lại. Khi bàn giao, ghi compiler, lệnh chạy, exit code,
seed kiểm thử và log kết quả. Source đã có không đồng nghĩa bài nộp hoàn tất:
cần hoàn thiện báo cáo PDF và đóng gói theo cấu trúc trong docs/Lab01.pdf.
