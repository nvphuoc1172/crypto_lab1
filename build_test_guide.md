# Hướng dẫn build và chạy test Lab01

Tài liệu áp dụng cho mã nguồn hiện có, kiểm tra ngày 04/11/2026. Chương trình test chính là `prng_sample/src/test_u512.cpp`, kiểm tra số lớn 512/1024 bit và lũy thừa mô-đun. Đây chưa phải chương trình trao đổi khóa Diffie–Hellman hoàn chỉnh.

## 1. Yêu cầu chuẩn bị

| Môi trường | Công cụ cần có |
| --- | --- |
| Windows | PowerShell, Command Prompt hoặc Git Bash; GCC/MinGW có `g++` và header Windows; thư mục `bin` của compiler nằm trong PATH |
| Linux/WSL | Linux, `g++`, GNU Make và các header phát triển hệ thống |

- Dùng C++14 theo lệnh bên dưới; không cần cài GMP, NTL hay framework kiểm thử bên ngoài.
- Thư viện SHAKE/SHA3 đã có trong `prng_sample/lib/fips202/`.
- Không thêm `-DNDEBUG`: các kiểm tra sử dụng `assert`, cờ này sẽ vô hiệu hóa chúng.
- Compiler hỗ trợ số nguyên 128 bit sẽ dùng nhánh tương ứng; compiler không hỗ trợ dùng nhánh dự phòng. Hai nhánh cần được kiểm tra riêng.
- Makefile dùng lệnh Unix; trên PowerShell nên dùng lệnh compiler trực tiếp thay vì chạy Makefile.
- Hiện không có `build.ps1` hoặc `tests/prng_test.cpp` trong bản source được kiểm tra. Các file `.exe` còn trên máy có thể thuộc bản build cũ, không dùng làm bằng chứng source hiện tại đã đúng.

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

## 2. Windows: build và chạy test số lớn

### Chọn đúng terminal

Windows Terminal và terminal tích hợp trong VS Code có thể mở nhiều shell khác nhau. Dùng cú pháp theo shell đang mở, không chỉ theo tên ứng dụng terminal.

| Shell | Dấu nhắc thường gặp | Xem thư mục hiện tại | Xem exit code ngay sau lệnh |
| --- | --- | --- | --- |
| PowerShell | `PS D:\...>` | `Get-Location` | `$LASTEXITCODE` (chương trình ngoài) |
| Command Prompt (CMD) | `D:\...>` | `cd` | `echo %ERRORLEVEL%` |
| Git Bash / Bash trên WSL | `$` | `pwd` | `echo $?` |

Trong VS Code, chọn profile terminal qua menu cạnh nút tạo terminal mới. Đổi shell không tự cài compiler: chạy `g++ --version` trong từng môi trường. Git Bash dùng compiler Windows có trên PATH; WSL cần compiler Linux riêng.

### PowerShell

Mở PowerShell tại thư mục gốc dự án:

```powershell
cd 'D:\C++\CryptoLab'
g++ --version 
cd .\prng_sample
New-Item -ItemType Directory -Force -Path .\bin | Out-Null
```

Build bằng lệnh sau (một dòng):

```powershell
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/test_u512.cpp src/u512_sample.cpp src/dh_sample.cpp -I src -I lib/fips202 -o bin/test-u512.exe
```

Kiểm tra kết quả và chỉ chạy nếu build thành công:

```powershell
if ($LASTEXITCODE -eq 0) {
    & .\bin\test-u512.exe
    Write-Host "Test exit code: $LASTEXITCODE"
} else {
    Write-Host 'Build thất bại; không chạy executable cũ.'
}
```

Test này không gọi PRNG nên không cần liên kết `prng.cpp`, `fips202.c` hoặc thư viện `advapi32`.

### Command Prompt (CMD)

Mở Command Prompt và chạy từng dòng:

```bat
cd /d "D:\C++\CryptoLab\prng_sample"
g++ --version
if not exist bin mkdir bin
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/test_u512.cpp src/u512_sample.cpp src/dh_sample.cpp -I src -I lib/fips202 -o bin/test-u512.exe && bin\test-u512.exe
echo Exit code: %ERRORLEVEL%
```

`cd /d` đổi cả ổ đĩa lẫn thư mục; dùng dấu ngoặc kép cho đường dẫn. `&&` chỉ chạy test khi build thành công. Gõ dòng `echo` riêng ngay sau dòng build/chạy để xem mã trả về. CMD không dùng `New-Item`, `$LASTEXITCODE` hoặc toán tử gọi `&` của PowerShell.

### Git Bash trên Windows

Git Bash cung cấp shell kiểu Bash nhưng không mặc định cung cấp `g++`. Sau khi compiler Windows đã có trên PATH:

```sh
cd '/d/C++/CryptoLab/prng_sample'
g++ --version
mkdir -p bin
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/test_u512.cpp src/u512_sample.cpp src/dh_sample.cpp -I src -I lib/fips202 -o bin/test-u512.exe && ./bin/test-u512.exe
echo "Exit code: $?"
```

Git Bash dùng đường dẫn `/d/...`; WSL dùng `/mnt/d/...`. Bộ lệnh trên tạo executable Windows khi dùng MinGW. Các ví dụ CMD/Git Bash dùng cùng danh sách source đã build bằng PowerShell; chưa được chạy xác nhận riêng trong từng shell.

## 3. Linux/WSL: build và chạy test số lớn

Nếu dùng WSL với dự án ở ổ D:

```sh
cd '/mnt/d/C++/CryptoLab/prng_sample'
g++ --version
make --version
make run-test-u512
```

Trên Linux khác, thay đường dẫn bằng vị trí checkout. Các target hiện có:

| Lệnh | Tác dụng |
| --- | --- |
| `make test-u512` | Build test số lớn vào `bin/test-u512` |
| `make run-test-u512` | Build rồi chạy test số lớn |
| `make prng` | Build demo PRNG |
| `make run-prng` | Build rồi chạy demo PRNG |
| `make clean` | Xóa toàn bộ thư mục `bin/` |

Nếu không dùng Make, có thể build và chạy trực tiếp:

```sh
mkdir -p bin
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/test_u512.cpp src/u512_sample.cpp src/dh_sample.cpp -I src -I lib/fips202 -o bin/test-u512 && ./bin/test-u512
```

Các lệnh Linux/WSL ở trên được đối chiếu với Makefile, chưa được chạy xác nhận trong lần kiểm tra Windows này.

## 4. Cách đọc kết quả test

Bộ test chạy tuần tự:

1. Cộng, trừ, carry/borrow và so sánh.
2. Dịch bit qua ranh giới limb.
3. Chuyển đổi chuỗi hex.
4. Nhân 512 × 512 → 1024 bit và phép lấy dư.
5. Lũy thừa mô-đun.

Khi tất cả kiểm tra thành công, chương trình in:

```text
=== ALL BIGINT TESTS PASSED SUCCESSFULLY! ===
```

Khi assertion thất bại, chương trình dừng tại kiểm tra đầu tiên lỗi; các phần sau chưa được kiểm chứng. Build thành công không đồng nghĩa thuật toán đúng.

### Lỗi đã tái hiện trên Windows

Với MinGW GCC 6.3.0 hiện có, build thành công nhưng test dừng tại `src/test_u512.cpp:92`:

```text
Assertion failed: prod.digits[1] == 0xFFFFFFFFFFFFFFFEULL
```

Ba nhóm kiểm tra đầu đã qua; nhóm nhân/lấy dư chưa qua và nhóm lũy thừa chưa chạy tới. Cần sửa phép nhân trong nhánh không có `__int128` ở `src/u512_sample.cpp`. Không xóa assertion để làm test báo thành công.

## 5. Demo PRNG trên Windows

Demo PRNG là chương trình riêng, không thay thế test số lớn. Từ `prng_sample/`:

```powershell
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/main_prng.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I lib/fips202 -o bin/prng.exe -ladvapi32
if ($LASTEXITCODE -eq 0) { & .\bin\prng.exe }
```

Lệnh liên kết `advapi32` vì bản PRNG hiện dùng `CryptAcquireContext`, `CryptGenRandom` và `CryptReleaseContext`. Bản hiện tại đã khác bản BCrypt trong trao đổi trước.

Nếu dùng CMD, từ `prng_sample/` chạy:

```bat
if not exist bin mkdir bin
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/main_prng.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I lib/fips202 -o bin/prng.exe -ladvapi32 && bin\prng.exe
echo Exit code: %ERRORLEVEL%
```

Nếu dùng Git Bash, từ cùng thư mục chạy:

```sh
mkdir -p bin
g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic src/main_prng.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I lib/fips202 -o bin/prng.exe -ladvapi32 && ./bin/prng.exe
echo "Exit code: $?"
```

Lưu ý trước khi dùng demo:

- MinGW cũ có thể báo lỗi tại `PRIu64` trong `main_prng.cpp`; cần điều chỉnh phần xuất số 64 bit cho compiler đó.
- Mảng seed hiện chưa được khởi tạo toàn bộ. Khi sửa code, dùng `uint8_t seed[32] = {};` trước khi chép seed cố định để tái hiện kết quả.
- Nhánh Windows hiện có fallback `rand()` khi lấy nguồn ngẫu nhiên thất bại; cần xử lý lại trước khi dùng cho sinh khóa.

## 6. Xử lý lỗi môi trường

| Triệu chứng | Cách xử lý |
| --- | --- |
| Không nhận diện `g++` | Kiểm tra cài đặt compiler, PATH; mở lại terminal sau khi đổi PATH |
| Không tìm thấy file `.cpp` | Chạy lệnh từ `prng_sample/`; kiểm tra bằng `Get-Location` (PowerShell), `cd` (CMD) hoặc `pwd` (Bash) |
| Không tìm thấy `prng.h` | Giữ tham số `-I lib/fips202` |
| Undefined reference tới hàm số lớn | Build kèm `src/u512_sample.cpp` và `src/dh_sample.cpp` |
| Undefined reference tới `Crypt...` | Với demo PRNG Windows hiện tại, thêm `-ladvapi32` cuối lệnh |
| Application Control chặn executable | Kiểm tra chính sách máy; đây là lỗi quyền chạy, chưa phải kết quả test thuật toán |
| Assertion failed | Đọc file/dòng báo lỗi và sửa thuật toán; không tắt `assert` |

Khi bàn giao, ghi phiên bản compiler, lệnh đã chạy, exit code và log lỗi. Chỉ commit source và tài liệu; executable được tạo lại tại máy người nhận.
