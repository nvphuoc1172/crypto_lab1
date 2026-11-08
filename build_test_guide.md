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

## Language standard

### Lưu ý về chuẩn C++ của đồ án

Đề bài chỉ cho phép C++14 trở xuống. Nhóm nên thống nhất **C++14** và ghi rõ `-std=c++14` trong mọi lệnh build. Cú pháp là `-std=`, không phải `std==`. Đây là lựa chọn lúc biên dịch; chạy file `.exe` không chọn lại chuẩn C++.

- C++11 có thể dùng nếu source tương thích, nhưng đề không yêu cầu phải hạ xuống C++11/C++98. C++14 cho phép các tiện ích như `std::make_unique` và lambda tổng quát.
- Không dùng tính năng từ C++17 như structured bindings (`auto [a, b] = ...`), `if constexpr`, `std::optional`, `std::filesystem`; hoặc C++20 như concepts, ranges và `std::span` trong bản nộp.
- Không chọn C++17 trở lên cho bản build nghiệm thu vì vượt giới hạn đề bài, có thể che giấu việc source phụ thuộc tính năng mới và không build được ở môi trường chấm C++14. Chuẩn mới không tự gây lỗi thuật toán; lý do ở đây là tuân thủ yêu cầu và giữ môi trường chung.
- Có thể dùng compiler mới nhưng vẫn chọn `-std=c++14`. Phiên bản compiler khác với phiên bản ngôn ngữ; không cần cố dùng compiler cũ.
- `-std=gnu++14` bật chế độ GNU mở rộng, khác với `-std=c++14`. Ngay cả `-std=c++14` cũng không bảo đảm mọi phần mở rộng đều bị từ chối; giữ `-Wpedantic` để xem cảnh báo. `__int128` là phần mở rộng compiler, không phải kiểu chuẩn C++14 hay C++17. Đổi sang C++17 không làm kiểu này khả dụng trên mọi kiến trúc.
- Cờ chuẩn không thay thế thư viện/API hệ điều hành: `windows.h`, `CryptGenRandom` và `getrandom` vẫn phụ thuộc nền tảng.

Tham khảo về lựa chọn chuẩn và GNU extensions: [tài liệu GCC](https://gcc.gnu.org/onlinedocs/gcc-15.1.0/gcc/C-Dialect-Options.html).

### Không ghi `-std` thì dùng chuẩn nào?

**Không có mặc định chung là C++11.** Mặc định phụ thuộc compiler, phiên bản và các cờ do IDE/build system truyền vào. Với GCC chính thức, các mốc thường dùng là:

| Phiên bản GCC | Chế độ C++ mặc định |
| --- | --- |
| GCC 6–10 | `gnu++14` |
| GCC 11–15 | `gnu++17` |
| GCC 16 | `gnu++20` |

Nguồn: [GCC 15 — mặc định gnu++17](https://gcc.gnu.org/onlinedocs/gcc-15.1.0/gcc/C-Dialect-Options.html), [GCC — lịch sử chế độ mặc định](https://gcc.gnu.org/onlinedocs/libstdc++/manual/using.html). Không áp bảng này cho MSVC/Clang hay compiler được tùy chỉnh; kiểm tra trực tiếp môi trường đang dùng.

Trên máy đã kiểm tra, MinGW GCC 6.3.0 không truyền `-std` cho giá trị `__cplusplus = 201402L`, tức mức C++14, với chế độ mặc định GNU. Vì vậy nhớ mặc định là C++11 không đúng với compiler hiện tại.

Kiểm tra trên PowerShell:

```powershell
g++ --version
g++ -dM -E -x c++ NUL | Select-String '__cplusplus'
g++ -std=c++14 -dM -E -x c++ NUL | Select-String '__cplusplus'
```

Trên CMD:

```bat
g++ --version
g++ -dM -E -x c++ NUL | findstr __cplusplus
g++ -std=c++14 -dM -E -x c++ NUL | findstr __cplusplus
```

Trên Git Bash/Linux/WSL:

```sh
g++ --version
g++ -dM -E -x c++ /dev/null | grep __cplusplus
g++ -std=c++14 -dM -E -x c++ /dev/null | grep __cplusplus
```

Các lệnh trên chỉ chạy tiền xử lý, không tạo executable. Giá trị thường gặp: `199711L` (C++98/03), `201103L` (C++11), `201402L` (C++14), `201703L` (C++17), `202002L` (C++20), `202302L` (C++23). Compiler cũ có thể báo giá trị tạm cho chuẩn đang thử nghiệm. Macro này không phân biệt chế độ ISO với GNU và không chứng minh toàn bộ source tuân thủ chuẩn.

Khi dùng VS Code, cấu hình IntelliSense chỉ ảnh hưởng phân tích mã; cần kiểm tra cả lệnh build thực tế hoặc Makefile để chắc chắn compiler nhận `-std=c++14`.
