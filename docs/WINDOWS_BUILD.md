# Hướng dẫn build và kiểm tra Lab01

Cập nhật 08/11/2026. Dùng C++14, GCC/MinGW trên Windows hoặc GCC trên Linux.
Không cần GMP/NTL; PRNG/SHAKE được cung cấp trong source. Không thêm `-DNDEBUG`
vì test sử dụng `assert`. Compiler phải nằm trong PATH (`g++ --version`).

## PowerShell

Từ thư mục gốc dự án:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\prng_sample\build.ps1 -Test -TestSafe512
.\prng_sample\bin\dh.exe
```

Bỏ hai cờ để chỉ build. `-Test` chạy demo PRNG và các test số lớn, private key,
nguyên tố, regression. `-TestSafe512` chạy thêm kiểm tra sinh safe prime 512 bit.
Script tự tạo `bin/`, dừng khi build hoặc test thất bại, không chạy executable cũ.
ExecutionPolicy chỉ áp dụng cho tiến trình gọi này, không đổi cấu hình máy.

## Command Prompt (CMD)

```bat
cd /d "D:\C++\CryptoLab"
powershell -NoProfile -ExecutionPolicy Bypass -File prng_sample\build.ps1 -Test -TestSafe512
echo Exit code: %ERRORLEVEL%
prng_sample\bin\dh.exe
```

Nếu có `mingw32-make`, thay script bằng:

```bat
cd /d "D:\C++\CryptoLab\prng_sample"
mingw32-make test
bin\test-prime.exe --safe512
bin\dh.exe
```

## Git Bash

Git Bash cần compiler Windows trên PATH; shell này không tự cài GCC.

```sh
cd '/d/C++/CryptoLab/prng_sample'
mingw32-make test && ./bin/test-prime.exe --safe512
echo "Exit code: $?"
./bin/dh.exe
```

## Linux/WSL

```sh
cd '/mnt/d/C++/CryptoLab/prng_sample'
make test && ./bin/test-prime --safe512
echo "Exit code: $?"
make run-dh
```

Trên Linux không dùng WSL, thay đường dẫn bằng vị trí checkout.
Chưa xác nhận runtime Linux/MSVC trong lần sửa Windows này; macOS chưa hỗ trợ entropy.

## Build riêng và đọc kết quả

Từ `prng_sample/`, trên Windows thay `make` bằng `mingw32-make`:

| Lệnh | Chức năng |
| --- | --- |
| `make all` | Build cả demo và test |
| `make test` | Build và chạy toàn bộ test thường |
| `make run-prng` | Build/chạy demo seed cố định |
| `make run-dh` | Build/chạy DH 512 bit, seed hệ điều hành |
| `make run-test-u512` | Test số lớn/lũy thừa |
| `make run-test-private-key` | Test khóa riêng và DH với prime cố định |
| `make run-test-prime` | Test Miller–Rabin và prime nhỏ |
| `make test-regressions` | Build test các lỗi đã sửa; chạy file tương ứng trong bin |

Exit code 0 là thành công. Assertion hoặc mã thoát khác 0 là lỗi; không tắt assert.
Thời gian sinh safe prime phụ thuộc seed và máy; khác biệt thời gian không tự có nghĩa là lỗi.
Private key có thể ngắn hơn 512 bit nhưng phải thuộc [2, p−2]. Demo có in khóa riêng để học tập.

## Thay đổi hợp đồng và lỗi thường gặp

- Modulus 0 ném `std::invalid_argument`; modulus 1 vẫn trả 0 khi tính mô-đun.
- `from_hex` chỉ nhận 1–128 ký tự hex (có thể có tiền tố 0x); dữ liệu sai bị từ chối.
- `add_mod` và `sub_mod` nhận đầu vào chưa rút gọn và trả kết quả đã rút gọn.
- `randombytes` tối đa 255 byte/lần, dừng nếu API hệ điều hành thất bại; không dùng rand().
- Demo DH kiểm tra phần tử sinh toàn nhóm với p=2q+1 và trả lỗi khi xác minh không đạt.
- Không thấy g++: sửa PATH và mở lại terminal. Không thấy source: kiểm tra thư mục hiện tại.
- Link thủ công cần có u512_sample.cpp, dh_sample.cpp, prng.cpp, fips202.c;
  thêm prime_sample.cpp khi gọi sinh nguyên tố; trên Windows liên kết `-ladvapi32`.
- Application Control chặn executable là lỗi chính sách thực thi, chưa phải lỗi thuật toán.

## Mẫu báo cáo

Script không tự tìm bản mẫu cũ hay ghi đè bản Word đã chỉnh tay. Chỉ chạy khi cần tạo lại:

```powershell
python docs/reports/create_report_template.py --source "duong-dan-mau-goc.docx" --output "mau-moi.docx"
```

Đường dẫn output phải chưa tồn tại. Mẫu sinh lại không thay thế các chỉnh sửa thủ công trên mẫu đang dùng.
