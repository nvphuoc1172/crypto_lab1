# Hướng dẫn build, kiểm tra và chạy Lab01 trên Windows và Linux

Cập nhật 08/11/2026. Dùng C++14, GCC/MinGW trên Windows hoặc GCC trên Linux.
Không cần GMP/NTL; PRNG/SHAKE được cung cấp trong source. Không thêm `-DNDEBUG`
vì test sử dụng `assert`. Compiler phải nằm trong PATH (`g++ --version`).

## Công cụ và cấu trúc chương trình

| Môi trường | Công cụ | Cách build chính |
| --- | --- | --- |
| Windows PowerShell/CMD | MinGW GCC có g++ trong PATH; PowerShell | `prng_sample/build.ps1` |
| Windows có GNU Make | MinGW GCC và mingw32-make | `mingw32-make` trong prng_sample/ |
| Linux/WSL | GCC có g++, GNU Make, header phát triển Linux | `make` trong prng_sample/ |

Kiểm tra `g++ --version`; nếu dùng Make, kiểm tra thêm `make --version` hoặc
`mingw32-make --version`. WSL cần compiler Linux riêng; Git Bash dùng compiler
Windows trên PATH. Không gom mọi file src/*.cpp vào một executable vì có nhiều main.

Khóa riêng được triển khai tại `src/private_key.cpp`, gồm cả overload tự seed.
Script PowerShell và Makefile đã liên kết module này, giữ các sửa lỗi số học/Windows.
`-Test` chạy cả hai bộ test khóa riêng, test PRNG, regression và DH `--test`.
Demo `dh.exe --test` dùng p=23 và in bí mật; `dh.exe` dùng 512 bit và chỉ thông báo
hai bí mật trùng nhau. Sinh seed cố định chỉ dành cho kiểm tra.

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

Trên Ubuntu/Debian, nếu chưa có compiler và Make:

```sh
sudo apt update
sudo apt install build-essential
```

Với distro khác, cài g++ và GNU Make bằng trình quản lý gói tương ứng.
Sau đó mở terminal Linux/WSL:

```sh
cd '/mnt/d/C++/CryptoLab/prng_sample'
g++ --version
make --version
make all
make test && ./bin/test-prime --safe512
echo "Exit code: $?"
./bin/dh --test
./bin/dh
```

Trên Linux không dùng WSL, thay đường dẫn bằng vị trí checkout.
Chưa xác nhận runtime Linux/MSVC trong lần sửa Windows này; macOS chưa hỗ trợ entropy.

`make all` chỉ build; `make test` build rồi chạy test thường. `--safe512` kiểm tra
sinh safe prime 512 bit; `dh --test` dùng p=23, còn `dh` dùng 512 bit.
Lệnh chạy test cuối thất bại sẽ cho exit code khác 0; đọc log trước khi chạy tiếp.

Windows tạo file `.exe`; Linux tạo executable không có phần mở rộng. Khi chuyển
qua lại hai môi trường, build lại bằng compiler của môi trường đích, không dùng
binary cũ để kết luận. Tránh build đồng thời vào cùng bin/ từ Windows và WSL.

## Build riêng và đọc kết quả

Từ `prng_sample/`, trên Windows thay `make` bằng `mingw32-make`:

| Lệnh | Chức năng |
| --- | --- |
| `make all` | Build cả demo và test |
| `make test` | Build và chạy toàn bộ test thường |
| `make run-prng` | Build/chạy demo seed cố định |
| `make run-dh` | Build/chạy DH 512 bit, seed hệ điều hành |
| `make test-dh` | Build/chạy demo DH nhanh với p=23 |
| `make run-test-u512` | Test số lớn/lũy thừa |
| `make run-test-private-key` | Test khóa riêng và DH với prime cố định |
| `make run-test-prime` | Test Miller–Rabin và prime nhỏ |
| `make test-regressions` | Build test các lỗi đã sửa; chạy file tương ứng trong bin |
| `make test-dh-exchange` | Build test biên DH; chạy bin/test-dh-exchange trên Linux hoặc bin/test-dh-exchange.exe trên Windows |
| `make clean` | Xóa đầu ra build; cần build lại trước khi chạy |

Exit code 0 là thành công. Assertion hoặc mã thoát khác 0 là lỗi; không tắt assert.
Thời gian sinh safe prime phụ thuộc seed và máy; khác biệt thời gian không tự có nghĩa là lỗi.
Private key có thể ngắn hơn 512 bit nhưng phải thuộc [2, p−2]. Demo không in khóa riêng;
chỉ `--test` in bí mật chung. Bộ test mở rộng có in khóa mẫu từ seed cố định.

## Thay đổi hợp đồng và lỗi thường gặp

- Modulus 0 ném `std::invalid_argument`; modulus 1 vẫn trả 0 khi tính mô-đun.
- `from_hex` chỉ nhận 1–128 ký tự hex (có thể có tiền tố 0x); dữ liệu sai bị từ chối.
- `add_mod` và `sub_mod` nhận đầu vào chưa rút gọn và trả kết quả đã rút gọn.
- `randombytes` tối đa 255 byte/lần, dừng nếu API hệ điều hành thất bại; không dùng rand().
- Demo DH kiểm tra phần tử sinh toàn nhóm với p=2q+1 và trả lỗi khi xác minh không đạt.
- Không thấy g++: sửa PATH và mở lại terminal. Không thấy source: kiểm tra thư mục hiện tại.
- Link thủ công demo cần có u512_sample.cpp, dh_sample.cpp, dh_exchange.cpp, private_key.cpp, prng.cpp, fips202.c;
  thêm prime_sample.cpp khi gọi sinh nguyên tố; trên Windows liên kết `-ladvapi32`.
- Application Control chặn executable là lỗi chính sách thực thi, chưa phải lỗi thuật toán.

## Trạng thái chạy và xử lý biên (08/11/2026)

MinGW GCC 6.3.0/C++14 đã build và chạy được các test số lớn, hai bộ test khóa riêng,
PRNG, nguyên tố, regression, safe prime 512 bit và demo DH. Các lần bị App Control
chặn trước đây là lịch sử kiểm tra, không còn là kết luận cho lần chạy thành công.
Linux/MSVC chưa được chạy xác nhận.

Demo gọi `perform_dh_exchange` trong `src/dh_exchange.cpp`. Nếu khóa công khai là
0, 1 hoặc p−1 (bao gồm khóa riêng bằng q), demo bỏ cả cặp và sinh lại; bí mật chung
0, 1 hoặc p−1 cũng bị loại. Tối đa 128 cặp để tránh lặp vô hạn; lỗi nguồn khóa hoặc
hai bí mật khác nhau trả lỗi ngay. Hàm `generate_private_key` vẫn giữ khoảng [2,p−2].
Không ép hai khóa riêng phải khác nhau. Đây là lọc bổ sung ở demo, không đổi API lấy mẫu.

Test `test-dh-exchange` đã được đưa vào `build.ps1 -Test` và `make test`.
Nó ép khóa bằng q, ép bí mật bằng 1 trong nhóm thử nghiệm không phải safe-prime,
kiểm tra giới hạn thử lại và đầu vào lỗi. Chỉ dùng nhóm safe-prime đã xác minh cho demo thực.

## Mẫu báo cáo (tạo lại khi cần)

Script không tự tìm bản mẫu cũ hay ghi đè bản Word đã chỉnh tay. Chỉ chạy khi cần tạo lại:

```powershell
python docs/reports/create_report_template.py --source "duong-dan-mau-goc.docx" --output "mau-moi.docx"
```

Đường dẫn output phải chưa tồn tại. Mẫu sinh lại không thay thế các chỉnh sửa thủ công trên mẫu đang dùng.

## Chuẩn ngôn ngữ

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
