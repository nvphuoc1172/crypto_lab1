# Module sinh và kiểm tra số nguyên tố

Tài liệu này mô tả module `prng_sample/src/prime_sample.cpp` và các khai báo
trong `prng_sample/src/dh_sample.h`.

## API và hợp đồng đầu vào/đầu ra

### `bool is_probable_prime(const uint512_t &n, prng_ctx *ctx, size_t rounds = 40)`

- **Đầu vào `n`:** số nguyên không dấu cần kiểm tra.
- **Đầu vào `ctx`:** PRNG đã được khởi tạo bằng `prng_seed`; được dùng để chọn
  cơ sở Miller–Rabin. `ctx` có thể null khi `n` là 2 hoặc 3; các trường hợp
  khác trả `false`.
- **Đầu vào `rounds`:** số vòng Miller–Rabin, mặc định 40. Giá trị 0 trả
  `false` cho các số lớn hơn 3.
- **Đầu ra:** `true` nghĩa là số nguyên tố theo phép thử xác suất; `false` nghĩa
  là hợp số hoặc đầu vào không hợp lệ theo các điều kiện trên. `true` không phải
  chứng minh nguyên tố tuyệt đối.

### `uint512_t generate_prime(prng_ctx *ctx, size_t bitsize = 512, size_t rounds = 40)`

- **Đầu vào `ctx`:** PRNG đã khởi tạo; không được null.
- **Đầu vào `bitsize`:** số bit cần sinh, từ 2 đến 512.
- **Đầu vào `rounds`:** số vòng Miller–Rabin cho mỗi ứng viên, phải lớn hơn 0.
- **Đầu ra:** probable-prime có đúng `bitsize` bit, luôn là số lẻ. Trả 0 nếu
  tham số không hợp lệ. Với tham số hợp lệ, hàm lặp đến khi tìm được ứng viên.

### `uint512_t generate_safe_prime(prng_ctx *ctx, size_t bitsize = 512)`

- **Đầu vào `ctx`:** PRNG đã khởi tạo; không được null.
- **Đầu vào `bitsize`:** số bit của `p`, từ 3 đến 512.
- **Đầu ra:** probable-prime `p` đúng số bit yêu cầu, cùng
  `q = (p - 1) / 2` cũng probable-prime. Mỗi phép thử dùng 40 vòng
  Miller–Rabin. Trả 0 nếu tham số không hợp lệ. Với tham số hợp lệ, hàm lặp
  đến khi tìm được cặp.

Các hàm sinh không nhận seed riêng; caller chịu trách nhiệm khởi tạo `ctx`.
Seed cố định phù hợp để tái hiện kiểm tra trong lớp, không phù hợp để bảo vệ
khóa thật.

## Cơ sở thuật toán và thư viện được dùng

Module tự cài đặt việc sinh và kiểm tra prime trong prime_sample.cpp; không gọi
thư viện ngoài để trả về số nguyên tố. Nền tảng số lớn và lũy thừa mô-đun cũng
do project tự cài trong u512_sample.cpp và dh_sample.cpp. Thành phần được tái
sử dụng từ mã mẫu là SHAKE128 ở lib/fips202/fips202.c cùng wrapper PRNG trong
lib/fips202/prng.cpp.

Luồng sinh số ngẫu nhiên cụ thể:

1. Caller cung cấp seed cho prng_seed. Hàm khởi tạo sponge SHAKE128, absorb
   seed rồi finalize trạng thái.
2. prng_next_u64 squeeze 8 byte từ SHAKE128 và ghép little-endian thành một
   limb 64 bit. random_bits ghép 8 limb thành ứng viên, cắt về độ dài yêu cầu,
   rồi đặt bit cao nhất và bit thấp nhất.
3. random_below dùng rejection sampling thay vì lấy dư trực tiếp để chọn cơ
   sở Miller-Rabin trong [2,n-2], tránh lệch modulo.

SHAKE128 là XOF được định nghĩa trong NIST FIPS 202. Wrapper trong project tạo
chuỗi xác định theo seed; nó không tự lấy entropy khi gọi hàm sinh prime.
randombytes() nằm cùng thư viện nhưng module prime hiện không gọi hàm đó. Vì
vậy seed kiểm thử cố định chỉ dành cho tái hiện kết quả; khi chạy ứng dụng thật,
caller phải cấp seed từ nguồn entropy phù hợp. Wrapper này phục vụ bài lab và
không được tuyên bố là DRBG đã được kiểm định theo NIST SP 800-90.

Cơ sở kiểm tra nguyên tố là Miller-Rabin: phân tích n-1 = d*2^s rồi thử các
cơ sở ngẫu nhiên để loại hợp số; sàng chia thử loại trước ứng viên có ước nhỏ.
FIPS 186-5 Appendix B.3 mô tả phép thử Miller-Rabin và Appendix B.7 mô tả lọc
chia thử trước phép thử xác suất. Đây là nguồn tham khảo cho kỹ thuật, không
phải tuyên bố module bài lab tuân thủ toàn bộ FIPS 186-5.

## Cơ chế kiểm tra nguyên tố

1. Loại `n < 2`, nhận ngay 2 và 3, loại số chẵn.
2. Chia thử qua các số nguyên tố lẻ từ 3 đến 251 để loại nhanh nhiều hợp số.
3. Viết `n - 1 = d * 2^s` với `d` lẻ.
4. Ở mỗi vòng, chọn đều một cơ sở `a` trong `[2, n-2]` bằng lấy mẫu từ chối,
   rồi tính `a^d mod n` bằng lũy thừa bình phương-nhân.
5. Tiếp tục bình phương modulo `n` tối đa `s-1` lần. Nếu không đạt điều kiện
   Miller–Rabin, kết luận hợp số; nếu qua tất cả vòng, trả probable-prime.

Với một hợp số lẻ và các cơ sở độc lập, đều trong `[2,n-2]`, xác suất một vòng
không phát hiện hợp số không quá `1/4`. Vì vậy `rounds=40` cho cận trên
`4^-40 = 2^-80` xác suất chấp nhận nhầm. Cận này phụ thuộc vào chất lượng seed
và tính độc lập giả ngẫu nhiên của PRNG.

## Cơ chế sinh

`generate_prime` lấy bit ngẫu nhiên, đặt bit cao nhất để đủ độ dài và đặt bit
thấp nhất để ứng viên là số lẻ; sau đó lặp kiểm tra cho đến khi được chấp nhận.

`generate_safe_prime` sinh `q` có `bitsize-1` bit, đặt bit cao nhất và bit thấp
nhất, rồi tạo ngay `p = 2q + 1`. Trước Miller–Rabin, sàng đồng thời `q` và `p`
bằng các số nguyên tố từ 3 đến 251; ứng viên bị loại nếu một trong hai chia hết
cho số nhỏ đó, trừ trường hợp chính nó bằng số nguyên tố đang xét. Ứng viên qua
sàng mới được kiểm tra Miller–Rabin cho cả `q` và `p`.

## Kiểm tra đã chạy

## Tối ưu nhân và modulo

Với modulus lẻ, `uint1024_t::mul_mod` và `modular_exponentiation` dùng Montgomery
reduction. Ngữ cảnh tính trước `-n[0]^-1 mod 2^32` và `R^2 mod n` một lần; sau
đó mỗi phép nhân modulo khử 16 limb cơ số 2^32, tránh chia bit-by-bit sau mỗi
bình phương trong Miller–Rabin. Với modulus chẵn, phép lũy thừa dùng đường dự
phòng chia modulo cũ.

Đường nhân dự phòng 512x512 cho compiler không có `__int128` dùng nhân schoolbook
16x16 limb 32 bit, với tích và carry trong `uint64_t`. Điều này sửa lỗi carry
đã bộc lộ khi thử ca biên 512 bit.

Kiểm tra được biên dịch bằng MinGW g++ với C++14; phần test dùng seed xác định.
Lệnh tương ứng trên Windows:

```powershell
g++ -std=c++14 -O3 -Wall -Wextra -Wpedantic src/test_prime.cpp src/prime_sample.cpp src/dh_sample.cpp src/u512_sample.cpp lib/fips202/prng.cpp lib/fips202/fips202.c -I src -I lib/fips202 -ladvapi32 -o bin/test-prime.exe
.\bin\test-prime.exe
```

Các ca nhỏ đã qua:

- Prime đã biết: 2, 3, 5, 7, 97, 65537.
- Hợp số: 0, 1, 4, 9, 25, 341, 561, 1105, 1729, `101 * 103` và `1009 * 1013`.
  Ca cuối có thừa số lớn hơn 251 nên đi vào nhánh Miller–Rabin sau sàng.
- Trường hợp đầu vào không hợp lệ: PRNG null, số vòng 0, kích thước prime không hợp lệ.
- Sinh prime 16 bit; seed kiểm tra cho kết quả `0xa9bb`.
- Sinh safe prime 5 bit; seed kiểm tra cho `p=0x17` và `q=0xb`.
- Sinh safe prime 3 bit (`p=7`, `q=3`) để kiểm tra ngoại lệ khi `q` bằng một
  trong các prime nhỏ của bảng sàng.
- Đối chiếu các phép nhân modulo với kết quả số học 64 bit cho modulus lẻ nhỏ;
  kiểm tra thêm `(n-1)^2 mod n = 1` với modulus 512 bit.
- Bộ test bigint sẵn có cũng qua: cộng/trừ, dịch, chuyển hex, nhân/modulo và
  lũy thừa mô-đun.

Để chạy ca 512 bit sau khi biên dịch, gọi `.\bin\test-prime.exe --safe512`.

### Thử nghiệm safe prime 512 bit

Sau tối ưu nhân/modulo và sàng đồng thời `q,p`, lần chạy cuối
`bin/test-prime-review.exe --safe512` sinh cặp 512 bit trong 0.45316 giây.
Hàm sinh dùng 40 vòng Miller–Rabin cho `p` và `q`; test xác
nhận lại độc lập cả hai với 8 vòng. Với seed test cố định, kết quả:

`p = 0x87345094e66da7782ed5ed9fe2d2c891cf4222938b8babbda2ee0d5ff262d9d2c873e174a40f107e76a2af9d3008e363945e9cbbdf5a9a082fd0cee5cc4f0a9f`

`q = 0x439a284a7336d3bc176af6cff1696448e7a11149c5c5d5ded17706aff9316ce96439f0ba5207883f3b5157ce980471b1ca2f4e5defad4d0417e86772e627854f`

## Lỗi và giới hạn phát hiện

Trong lần test ban đầu, helper chia thử trả nhầm trạng thái cho một số nguyên tố
nằm trong danh sách chia thử (chẳng hạn 5); đã sửa và các ca số nguyên tố nhỏ qua.
Đường nhân dự phòng không có `__int128` cũng từng sai ở ca lớn do carry; đã
thay bằng nhân schoolbook 32 bit và ca biên 512 bit qua sau sửa. Sau khi tối ưu,
chưa ghi nhận lỗi kết quả trong các test đã chạy. Các hàm sinh vẫn không giới hạn
số lần thử; với đầu vào hợp lệ, thời gian hoàn tất không được đảm bảo tuyệt đối.

## Tài liệu tham khảo

1. NIST, [FIPS 202: SHA-3 Standard, including SHAKE XOFs](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.202.pdf).
2. NIST, [FIPS 186-5: Digital Signature Standard, Appendix B.3 and B.7](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.186-5.pdf).
3. NIST, [SP 800-90A Rev. 1: Deterministic Random Bit Generators](https://csrc.nist.gov/pubs/sp/800/90/a/r1/final). Dùng làm nguồn đọc thêm để phân biệt luồng giả ngẫu nhiên xác định theo seed với nguồn entropy; không dùng để tuyên bố wrapper SHAKE của bài là DRBG tuân chuẩn.
