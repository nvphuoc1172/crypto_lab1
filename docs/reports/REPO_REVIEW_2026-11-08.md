# Kết quả rà soát repository — 08/11/2026

> Đây là bản ghi trước khi sửa. Sau rà soát, đã sửa nguồn entropy, seed/demo PRNG,
> phép dư uint64_t, chuẩn hóa cộng/trừ mô-đun, parser, kiểm tra g và exit code DH,
> build Windows, hướng dẫn và đường dẫn script báo cáo. Bộ regression mới ở
> `prng_sample/src/test_regressions.cpp`; lệnh hiện hành nằm trong `build_test_guide.md`.
> Hằng số wide_mod trong test_prime trước đây có 130 chữ số hex và bị cắt âm thầm;
> đã đổi thành hằng đúng 128 chữ số sau khi parser bắt đầu kiểm tra độ dài.

## Phạm vi và môi trường

Đọc các module C/C++, Makefile, test, script tạo báo cáo và tài liệu hướng dẫn. Kiểm tra động trên Windows, MinGW GCC 6.3.0 (nhánh không có `__int128`), C++14, bật cảnh báo. Không thay đổi mã nguồn ứng dụng. Không thể bảo đảm phát hiện mọi lỗi chỉ bằng các lần chạy này.

Linux/WSL, MSVC, compiler 64 bit có `__int128`, sanitizer và đường lỗi của API hệ điều hành chưa được chạy xác nhận. Các cảnh báo về nhánh đó là kết quả đọc mã, không phải lỗi runtime đã tái hiện.

## Kết quả kiểm tra

| Kiểm tra | Kết quả |
| --- | --- |
| Test số lớn hiện có | Build và chạy qua toàn bộ |
| Test private key hiện có | Build và chạy qua toàn bộ, gồm DH 512 bit |
| Test nguyên tố mặc định | Build và chạy qua |
| Test nguyên tố `--safe512` | Qua; sinh và kiểm tra lại p/q, khoảng 0,82 giây với seed cố định |
| Demo DH | Lần chạy ngay trước đợt rà soát đã thành công, hai bí mật trùng nhau; chưa chạy lại sinh prime ngẫu nhiên trong đợt này |
| Demo PRNG | Build thất bại tại `PRIu64` trên compiler hiện tại |
| `mingw32-make test-private-key` | Lỗi target `folders`: `mkdir -p` không hợp shell CMD |
| Đối chiếu số lớn với Python | 60 bộ số 512 bit cho cộng, trừ, nhân, nhân mô-đun và 8 ca lũy thừa đều qua |
| Các ca biên bổ sung | Tái hiện 3 sai khác: `% uint64_t`, add_mod/sub_mod với đầu vào chưa rút gọn |
| Script tạo báo cáo | FileNotFoundError do đường dẫn sau khi di chuyển script |
| Git | Không thấy marker conflict hoặc lỗi `git diff --check`; còn binary Linux được track |

## 1. Cần ưu tiên sửa

### 1.1. Nguồn entropy có thể thất bại nhưng chương trình vẫn sinh khóa

Vị trí: `prng_sample/lib/fips202/prng.cpp:10–17`.

- Không kiểm tra giá trị trả về của `CryptGenRandom`. Nếu API thất bại, seed có thể chứa byte chưa được ghi.
- Nếu `CryptAcquireContext` thất bại, hàm dùng `rand()` thay thế. Khóa vẫn có thể nằm đúng khoảng nhưng nguồn ngẫu nhiên không đạt yêu cầu sử dụng cho mật mã.
- Hàm overload `generate_private_key(p)` và demo DH phụ thuộc đường này.

Đề xuất: kiểm tra mọi kết quả API, trả lỗi rõ ràng hoặc dừng; không tự hạ xuống `rand()`. Chưa ép lỗi API để kiểm chứng nhánh thất bại.

### 1.2. `% uint64_t` sai với modulus lớn trên nhánh portable

Vị trí: `prng_sample/src/u512_sample.cpp:364–365`.

`rem << 32` có thể mất bit trước khi lấy dư. Ca đã tái hiện:

```text
a = 2^64
m = 2^64 - 1
Kết quả đúng: a % m = 1
Nhánh portable hiện tại: 0
```

Lọc prime nhỏ hiện dùng modulus tối đa 251 nên ca này chưa làm hỏng đường lọc đó. Tuy nhiên giao diện nhận toàn bộ uint64_t nên cần sửa hoặc giới hạn/ghi rõ hợp đồng.

### 1.3. Demo PRNG không build trên MinGW hiện tại và seed chưa khởi tạo đủ

Vị trí: `prng_sample/src/main_prng.cpp:8,22`.

- Lỗi đã tái hiện: `expected ')' before 'PRIu64'`.
- `seed[32]` chỉ được chép chuỗi `seed` và byte kết thúc; các byte còn lại chưa khởi tạo nhưng vẫn được đọc vào SHAKE.

Đề xuất: dùng cách xuất uint64_t tương thích compiler, khởi tạo toàn bộ seed trước khi chép. Seed cố định chỉ phục vụ học tập/kiểm tra.

## 2. Lỗi build và tài liệu

- `Makefile:29`: `mkdir -p` khiến mingw32-make với CMD thất bại trước bước compiler. Cần nhánh lệnh theo shell/nền tảng hoặc ghi rõ chỉ chạy Makefile trong môi trường Unix.
- `Makefile:34`: target `prng` thiếu `$(PRNG_LIBS)` trong khi `prng.cpp` gọi CryptoAPI Windows. Sau khi sửa lỗi PRIu64 cần bảo đảm liên kết advapi32 như các target còn lại.
- `build_test_guide.md`: lệnh build test-u512 chưa thêm `prng.cpp`, `fips202.c` và thư viện Windows. `dh_sample.cpp` hiện đã gọi PRNG nên danh sách source cũ không đủ liên kết. Tài liệu cũng còn ghi lỗi nhân tại dòng 92 dù test đó hiện đã qua.
- `AGENTS.md` mô tả các header chưa triển khai, chưa có test/target DH và chưa có Git metadata; các thông tin này đã cũ.
- `docs/reports/create_report_template.py:6–8`: ROOT trỏ đến `docs/` sau khi script được chuyển thư mục; file nguồn không tồn tại ở đường dẫn đang dùng. Script dừng trước khi tạo file. Ngay cả khi sửa đường dẫn, cần cân nhắc việc chạy lại có ghi đè mẫu Word đã được chỉnh tay hay không.
- `prng_sample/bin/prng` vẫn được Git theo dõi. Quy tắc ignore `.exe` không loại binary Linux không có phần mở rộng. Không tự gỡ file trong đợt kiểm tra này.

## 3. Hợp đồng API và tình huống chưa được bảo vệ

### Cộng/trừ mô-đun cần đầu vào đã rút gọn

Vị trí: `u512_sample.cpp:906,916`; header không nêu tiền điều kiện.

```text
add_mod(20, 20, 7): hiện 33, đúng theo tên phép toán là 5
sub_mod(20, 1, 7): hiện 19, đúng theo tên phép toán là 5
```

Các hàm phù hợp nếu hợp đồng yêu cầu `0 <= a,b < mod`, `mod > 0`. Cần ghi rõ/kiểm tra điều kiện hoặc chuẩn hóa đầu vào. Montgomery hiện gọi add_mod với các giá trị đã rút gọn, nên không kết luận rằng đường Montgomery đang sai từ các ca này.

### Parse hex chấp nhận dữ liệu sai âm thầm

`from_hex("0x1g2")` trả `0x12`; chuỗi quá 512 bit bị cắt. Nếu chỉ dùng hằng số nội bộ có thể kiểm soát đầu vào, nhưng khi đọc từ người dùng/file nên báo lỗi hoặc ghi rõ hành vi.

### Modulus 0 không báo lỗi

Phép `%`, `uint1024_t::mod` và lũy thừa trả 0 với modulus 0. Đây là hành vi đã có trong code, cần quy định rõ vì dễ che lỗi truyền tham số.

### Nhánh Linux và macOS

Nhánh getrandom lặp vô hạn nếu trả 0 khi còn byte cần lấy. Ngoài ra chú thích nhận hỗ trợ macOS nhưng nhánh ngoài Windows vẫn dùng sys/random.h/getrandom của Linux; chưa có triển khai riêng cho macOS.

## 4. Diffie–Hellman và chất lượng test

- `main_dh.cpp:29` cố định g=2, không kiểm tra bậc/phần tử sinh. Với p cố định trong test, Python cho `2^((p-1)/2) mod p = 1`: g không sinh toàn bộ nhóm nhân, dù có thể dùng nhóm con bậc q theo thiết kế phù hợp. Cần chọn và mô tả rõ thiết kế, không gọi g=2 là phần tử sinh toàn nhóm một cách mặc định.
- `main_dh.cpp:61` luôn trả exit code 0 kể cả khi hai bí mật khác nhau. Cần trả mã lỗi khi xác minh thất bại và kiểm tra tham số sinh thành công trước các bước tiếp theo.
- Các test `auto_key1 != auto_key2` không phải điều kiện toán học bắt buộc; hai lần lấy mẫu độc lập vẫn có thể trùng (xác suất rất nhỏ ở đây).
- Thông báo phân phối “strictly uniform without modulo bias” mạnh hơn bằng chứng 20.000 mẫu. Rejection sampling giải thích tính đều; thống kê mẫu chỉ là smoke test.
- Nên thêm regression cho lỗi `%` vừa tái hiện, tiền điều kiện add_mod/sub_mod, parser, lỗi entropy và mã thoát demo. Không tắt assert hoặc xóa kiểm tra để che lỗi.

## Thứ tự xử lý đề xuất

1. Sửa xử lý lỗi entropy, seed và demo PRNG.
2. Sửa `% uint64_t`; chốt hợp đồng phép toán mô-đun.
3. Đồng bộ Makefile và hướng dẫn build với source hiện tại.
4. Kiểm tra/chọn g, mã thoát và xử lý lỗi demo DH.
5. Bổ sung regression, cập nhật tài liệu và đường dẫn script báo cáo.

Các executable kiểm tra và `review_checks.cpp` đối chiếu Python được tạo trong `prng_sample/bin/`. Không đưa các artifact này vào bài nộp thay cho source/test chính thức.
