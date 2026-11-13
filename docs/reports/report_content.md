# Tổng quan bài Lab

## Mục tiêu và phạm vi

Lab01 của môn Nhập môn Mã hóa - Mật mã yêu cầu xây dựng các thành phần số học phục vụ trao đổi khóa Diffie–Hellman: xử lý số nguyên lớn, tính lũy thừa mô-đun, sinh số nguyên tố an toàn, sinh khóa riêng và xác minh bí mật chung giữa hai bên. Chương trình sử dụng C++14 và làm việc với modulus tối thiểu 512 bit theo đề bài.

Nhóm triển khai kiểu số nguyên cố định 512 bit và vùng trung gian 1024 bit; tự xây dựng kiểm tra Miller–Rabin và quá trình tìm safe prime. Thư viện SHAKE128 có sẵn trong mã mẫu được sử dụng làm nguồn giả ngẫu nhiên sau khi khởi tạo bằng seed. Các phép toán số lớn và việc sinh nguyên tố không giao cho thư viện số lớn bên ngoài.

Ứng dụng minh họa mô phỏng Alice và Bob trong một tiến trình. Việc trao đổi giá trị công khai được thể hiện bằng truyền giá trị giữa các bước tính toán. Phạm vi của bài không bao gồm truyền mạng, chứng thư số, xác thực đối tác hoặc xây dựng một giao thức bảo mật hoàn chỉnh.

## Kiến trúc chương trình

| Thành phần | Vai trò |
| --- | --- |
| `u512_sample.h`, `u512_sample.cpp` | Biểu diễn số lớn; cộng, trừ, dịch, so sánh; nhân và phép toán mô-đun; Montgomery. |
| `dh_sample.h`, `dh_sample.cpp` | Khai báo API và cài đặt lũy thừa mô-đun. |
| `prime_sample.cpp` | Miller–Rabin, sinh probable prime và safe prime. |
| `private_key.cpp` | Lấy mẫu khóa riêng trong khoảng hợp lệ, có overload nhận seed từ hệ điều hành. |
| `dh_exchange.h`, `dh_exchange.cpp` | Tính khóa công khai, bí mật chung và xử lý cặp khóa suy biến. |
| `main_dh.cpp` | Điều phối demo; kiểm tra tham số; chọn phần tử sinh; hiển thị kết quả. |
| `lib/fips202/` | SHAKE/SHA3 và các hàm PRNG từ mã mẫu. |
| `build.ps1`, `Makefile` | Build, chạy demo và các bộ kiểm tra. |

Luồng xử lý bắt đầu từ seed hệ điều hành, sinh tham số, chọn phần tử sinh, lấy mẫu khóa riêng, tính khóa công khai, tính bí mật chung rồi xác minh hai kết quả bằng nhau. Cách chia module giúp kiểm tra số học độc lập với giao thức và tái hiện các tình huống biên.

# Hệ thống xử lý số lớn

## Biểu diễn dữ liệu

Kiểu `uint512_t` biểu diễn số nguyên không dấu bằng mảng `uint64_t digits[8]`. Thứ tự limb là little-endian: `digits[0]` chứa 64 bit thấp nhất và `digits[7]` chứa 64 bit cao nhất. Miền biểu diễn là từ `0` đến `2^512 - 1`.

Giá trị số được xác định theo biểu thức:

```text
x = digits[0] + digits[1] * 2^64 + ... + digits[7] * 2^448
```

Kiểu `uint1024_t` có 16 limb 64 bit, dùng để giữ đầy đủ tích của hai số 512 bit. Đây là hệ thống số nguyên cố định độ rộng; tăng tham số sinh prime lên trên 512 bit không tự mở rộng kiểu dữ liệu.

## Khởi tạo, so sánh và thao tác bit

Constructor mặc định khởi tạo toàn bộ limb bằng `0`. Constructor nhận `uint64_t` gán giá trị vào limb thấp nhất và xóa các limb còn lại. So sánh bắt đầu từ limb cao nhất; cặp limb khác nhau đầu tiên quyết định quan hệ giữa hai số.

Các hàm `bit(i)` và `set_bit(i, value)` định vị limb bằng `i / 64`, vị trí trong limb bằng `i % 64`. Chỉ số hợp lệ của số 512 bit nằm trong `[0, 511]`; ngoài miền, hàm đọc trả `false`, hàm ghi không làm gì. Hàm `bit_length()` duyệt từ limb cao để tìm bit có giá trị cao nhất; số không có độ dài bit bằng `0`.

Phép dịch tách độ dịch thành phần nguyên limb và phần bit. Với độ dịch từ 512 trở lên, kết quả của `uint512_t` bằng `0`; bit vượt ra khỏi miền biểu diễn bị bỏ. Nhánh ghép bit giữa hai limb chỉ thực hiện khi độ dịch trong limb khác `0`, tránh dịch một giá trị 64 bit đúng 64 vị trí.

## Cộng, trừ và xử lý tràn

Phép cộng duyệt từ limb thấp lên limb cao và truyền carry. Phép trừ truyền borrow theo cùng chiều. Khi compiler hỗ trợ số nguyên 128 bit, giá trị trung gian được giữ bằng phần mở rộng `__int128`; nhánh portable dùng so sánh tràn của các phép toán không dấu.

Hai phép toán có ngữ nghĩa modulo `2^512`. Carry hoặc borrow cuối cùng bị bỏ, không tạo thông báo lỗi. Ví dụ, cộng `1` vào giá trị lớn nhất cho `0`; lấy `0 - 1` cho giá trị lớn nhất. Đây là hành vi đã quy định và có test riêng. Các phép toán mô-đun mật mã không được thay bằng phép nhân đã mất phần cao rồi mới lấy dư.

## Nhân, lấy dư và chuyển đổi

Phép nhân `uint1024_t::mul` tạo tích 1024 bit. Nhánh portable tách hai toán hạng thành các limb 32 bit và nhân schoolbook; mỗi tích, giá trị ô đích và carry được cộng trong kiểu 64 bit. Với hai số có độ dài bit tối đa 512, cách này cần số lượng phép nhân limb tăng theo bình phương số limb.

Hàm `uint1024_t::mod` lấy dư bằng cách duyệt các bit từ cao xuống thấp, cập nhật phần dư và trừ modulus khi cần. Phép `%` với modulus `uint64_t` có đường tính nhanh cho modulus nhỏ và đường tránh tràn cho modulus 64 bit lớn trên compiler không có kiểu 128 bit.

Các hàm `add_mod` và `sub_mod` chuẩn hóa cả khi đầu vào chưa nhỏ hơn modulus. Modulus bằng `0` bị từ chối bằng `std::invalid_argument`. Hàm `from_hex` nhận từ 1 đến 128 chữ số hex, có thể có tiền tố `0x` hoặc `0X`; chuỗi sai ký tự, rỗng hoặc dài quá giới hạn cũng bị từ chối. Hàm `to_hex` xuất chữ thường có tiền tố và không giữ số không thừa ở đầu.

# Lũy thừa mô-đun

## Bài toán và thuật toán

Hàm `modular_exponentiation(base, exp, mod)` tính `base^exp mod mod`. Không tính toàn bộ lũy thừa rồi lấy dư vì kích thước kết quả trung gian có thể tăng rất lớn. Chương trình dùng bình phương và nhân theo biểu diễn nhị phân của số mũ.

```text
result = 1 mod mod
cur = base mod mod
for each exponent bit from highest to lowest:
    result = (result * result) mod mod
    if current bit is 1:
        result = (result * cur) mod mod
return result
```

Sau mỗi bước, kết quả tích lũy bằng lũy thừa của cơ số ứng với phần tiền tố của số mũ đã xử lý. Bình phương tương ứng với dịch tiền tố sang trái một bit; nhân thêm cơ số tương ứng với bit tiếp theo bằng `1`. Với số mũ có độ dài `k` bit, số phép nhân mô-đun thuộc bậc `O(k)`.

## Tối ưu Montgomery

Với modulus lẻ lớn hơn `1`, hàm tạo ngữ cảnh `uint512_montgomery_t`. Chọn `R = 2^512`; số trong miền Montgomery có biểu diễn `x * R mod mod`. Sau khi chuyển cơ số và giá trị đơn vị vào miền này, toàn bộ vòng bình phương/nhân sử dụng `multiply_montgomery`; cuối cùng chuyển kết quả ra bằng `from_montgomery`.

Do modulus lẻ nên nguyên tố cùng nhau với `R`. Phần giảm Montgomery trong chương trình dùng radix `2^32`, hệ số nghịch đảo theo limb thấp và mảng trung gian đủ cho carry. Giá trị `R^2 mod mod` được chuẩn bị khi tạo ngữ cảnh. Ngữ cảnh được tái sử dụng trong một lần lũy thừa để giảm chi phí so với thực hiện phép chia lấy dư lớn ở từng bước.

Với modulus chẵn, chương trình dùng đường nhân đầy đủ rồi lấy dư. Việc hỗ trợ modulus chẵn cần cho kiểm tra số học tổng quát, dù tham số prime trong DH là số lẻ.

## Điều kiện biên

Modulus `0` gây ngoại lệ. Modulus `1` luôn cho kết quả `0`. Khi số mũ bằng `0` và modulus lớn hơn `1`, hàm trả `1` theo quy ước lũy thừa. Cơ số được rút gọn trước khi dùng. Các ca đã đưa vào test gồm `3^10 mod 1000 = 49`, `2^65536 mod 65537 = 1`, cùng nhiều tổ hợp cơ số, số mũ và modulus nhỏ đối chiếu với phép toán máy.

# Kiểm tra nguyên tố và sinh số nguyên tố an toàn

## Cơ sở và lựa chọn kiểm tra

Một số nguyên tố chỉ có hai ước dương là đơn vị và chính nó. Chia thử đến căn bậc hai phù hợp cho số nhỏ nhưng không thích hợp với ứng viên 512 bit. Module dùng phép chia thử bằng danh sách prime nhỏ để loại nhanh ứng viên rõ ràng là hợp số; sau đó dùng Miller–Rabin.

Miller–Rabin kiểm tra ứng viên lẻ bằng cách viết `n - 1 = d * 2^s` với `d` lẻ. Với cơ sở `a`, tính `x = a^d mod n`; một vòng chấp nhận nếu `x` bằng `1`, bằng `n-1`, hoặc một lần bình phương tiếp theo cho `n-1`. Nếu không đạt, cơ sở đó chứng minh `n` là hợp số. Việc qua nhiều vòng chỉ cho kết luận probable prime, không phải chứng minh nguyên tố tuyệt đối [1].

## Cài đặt Miller–Rabin

Hàm `is_probable_prime(n, ctx, rounds)` xử lý trước các giá trị nhỏ, số chẵn và chia hết cho prime trong bảng. Với ứng viên còn lại, hàm chọn cơ sở trong `[2, n-2]` bằng rejection sampling, rồi chạy kiểm tra nhân chứng. Giá trị mặc định là 40 vòng.

Theo tính chất Miller–Rabin, với một hợp số lẻ cố định và các cơ sở độc lập, phân bố đều, xác suất qua nhầm mọi vòng không quá `4^(-rounds)`, tương ứng `2^-80` khi chạy 40 vòng. Đây là cận cho mô hình phép thử, không được diễn giải thành bảo đảm tuyệt đối của cả chương trình hoặc tỷ lệ lỗi hậu nghiệm của mọi số đã sinh. Tính đúng của phép toán nền và chất lượng lấy mẫu vẫn là điều kiện cần.

## Sinh prime và safe prime

Hàm `generate_prime` tạo số ngẫu nhiên đủ số bit yêu cầu, đặt bit cao nhất và bit thấp nhất bằng `1`, rồi kiểm tra nguyên tố. Hàm nhận kích thước từ 2 đến 512 bit; tham số ngoài miền, context rỗng hoặc số vòng bằng `0` trả số không.

Safe prime có dạng `p = 2q + 1`, trong đó cả `p` và `q` nguyên tố. Để sinh modulus có `b` bit, chương trình tạo `q` với `b-1` bit, buộc bit cao và bit lẻ, lập `p`, sàng đồng thời hai số bằng danh sách prime nhỏ, rồi chạy 40 vòng Miller–Rabin cho từng số. Hàm trả khi cả hai đều qua phép thử và `p` có đúng độ dài yêu cầu.

```text
repeat:
    q = random odd integer with b-1 bits
    p = 2*q + 1
    reject if the pair fails the small-prime sieve
until MillerRabin(q, 40) and MillerRabin(p, 40)
return p
```

Module hỗ trợ safe prime từ 3 đến 512 bit để có thể kiểm tra bằng ví dụ nhỏ; bản demo mặc định chọn 512 bit theo đề. Các vòng tìm prime chưa có giới hạn thời gian cố định, nên thời gian hoàn tất thay đổi theo seed và máy.

## Seed và tính ngẫu nhiên

Context `prng_ctx` chứa trạng thái SHAKE128. Hàm `prng_seed` khởi tạo, hấp thụ seed và chốt giai đoạn hấp thụ; các hàm lấy số đọc tiếp luồng đầu ra theo thứ tự byte little-endian. SHAKE128 là hàm đầu ra mở rộng được mô tả trong FIPS 202 [2]. Seed giống nhau và thứ tự lời gọi giống nhau tạo cùng luồng, hữu ích để tái hiện test.

Chế độ mặc định của demo lấy 32 byte seed bằng CryptoAPI trên Windows hoặc `getrandom` trên Linux. Khi API thất bại, chương trình báo lỗi hoặc dừng, không thay bằng `rand()`. Seed cố định chỉ dùng trong test. Wrapper SHAKE của bài không được tuyên bố là một DRBG đã được kiểm định theo chuẩn riêng; có sẵn SHAKE không đồng nghĩa đã có quản lý khóa an toàn cho sản phẩm thực tế.

# Trao đổi khóa Diffie–Hellman

## Tham số và công thức

Alice và Bob thống nhất prime `p` và phần tử sinh `g`. Alice giữ riêng `a`, Bob giữ riêng `b`; hai giá trị được lấy trong `[2, p-2]`. Công thức trao đổi là:

```text
Alice: A = g^a mod p
Bob:   B = g^b mod p
Alice computes s_A = B^a mod p
Bob computes   s_B = A^b mod p
s_A = s_B = g^(a*b) mod p
```

Các khóa công khai có thể được trao đổi; khóa riêng không được truyền cho bên còn lại. Main xác nhận lại tính probable prime của cả `p` và `q = (p-1)/2`. Với safe prime, các bậc nhóm con có thể là `1`, `2`, `q` và `2q`; điều kiện `g^2 mod p != 1` và `g^q mod p != 1` loại các bậc nhỏ hơn để chọn phần tử sinh toàn nhóm.

## Sinh khóa riêng

Hàm `generate_private_key(ctx, p)` yêu cầu context đã seed và `p >= 5`; nó không tự kiểm tra prime. Khoảng chứa `p-3` giá trị. Đặt `range = p-3`, lấy đủ `bit_length(range-1)` bit, loại ứng viên từ `range` trở lên rồi cộng `2`. Kết quả thuộc `[2,p-2]` và tránh modulo bias trong mô hình bit đầu ra phân bố đều.

Overload `generate_private_key(p)` tự lấy seed hệ điều hành và tạo context cục bộ. Hai lần lấy mẫu vẫn có thể cho cùng giá trị; yêu cầu của API không ép hai khóa phải khác nhau. Khóa riêng cũng không bắt buộc có đúng 512 bit; số bit của modulus và miền giá trị khóa là hai điều kiện khác nhau.

## Module trao đổi và xử lý biên

Hàm `perform_dh_exchange` trong `dh_exchange.cpp` nhận context, modulus, phần tử sinh và cấu trúc kết quả. Bên gọi chịu trách nhiệm xác minh prime, safe prime và bậc của phần tử sinh. Hàm kiểm tra giới hạn đầu vào, tạo hai khóa riêng, tính hai khóa công khai và bí mật chung.

Khóa riêng bằng `q` vẫn thuộc khoảng hợp lệ nhưng tạo khóa công khai `p-1`. Demo loại cả cặp và lấy lại nếu khóa công khai hoặc bí mật chung ngoài `[2,p-2]`, gồm các giá trị `0`, `1`, `p-1`. Mỗi lần gọi thử tối đa 128 cặp. Hết lượt thì trả `false` và xóa kết quả đầu ra. Nếu nguồn khóa trả giá trị ngoài khoảng hoặc hai bí mật khác nhau, hàm báo thất bại ngay để không che lỗi nguồn hay lỗi số học bằng việc thử lại.

Trong test, con trỏ hàm nguồn khóa được thay bằng nguồn xác định để ép các tình huống hiếm. Ca bí mật chung bằng `1` sử dụng prime `31`, có `(31-1)/2 = 15` là hợp số, chỉ nhằm đi qua nhánh bảo vệ; demo thật luôn xác minh safe prime trước khi gọi module.

## Ứng dụng thực tế và giới hạn bảo mật

Diffie–Hellman tạm thời trên trường hữu hạn là một cơ chế thỏa thuận bí mật trong TLS. RFC 7919 định nghĩa các nhóm FFDHE và cách thương lượng tham số đã biết cấu trúc; nhóm nhỏ nhất trong danh sách này là 2048 bit [3]. Chương trình 512 bit ở bài lab đáp ứng kích thước tối thiểu của đề, không tương đương cấu hình FFDHE dành cho triển khai thực tế.

Trong TLS 1.3, giá trị bí mật từ trao đổi khóa được đưa vào cơ chế dẫn xuất khóa, đồng thời kết hợp xác thực và kiểm tra thông điệp bắt tay [4]. Ý tưởng liên hệ với bài lab là hai bên tạo cùng dữ liệu bí mật từ khóa riêng và giá trị công khai. Chương trình hiện tại chưa triển khai các bước bao quanh đó, nên chỉ kiểm chứng phần toán học của trao đổi khóa. DH thuần túy không tự ngăn tấn công xen giữa nếu không có xác thực.

# Hướng dẫn biên dịch và chạy chương trình

## Môi trường và cấu hình

Mã nguồn được biên dịch bằng C++14; các lệnh sử dụng `-std=c++14`, bật cảnh báo và không định nghĩa `NDEBUG`. Windows cần GCC/MinGW trên PATH và liên kết thư viện hệ thống `advapi32`; Linux/WSL cần GCC cùng GNU Make. SHAKE/SHA3 được cung cấp trong source, không cần cài thư viện số lớn bên ngoài.

Phiên bản compiler và phiên bản ngôn ngữ là hai lựa chọn khác nhau. Có thể dùng GCC mới nhưng vẫn chọn C++14; không dùng tính năng C++17 trở lên trong bản nộp. Phần mở rộng `__int128` chỉ xuất hiện khi compiler hỗ trợ; nhánh portable được cung cấp để không phụ thuộc hoàn toàn vào kiểu này. Khả năng chạy trên MSVC và macOS chưa được xác nhận.

## Windows PowerShell và CMD

Từ thư mục gốc dự án, kiểm tra compiler và build toàn bộ kèm test:

```powershell
g++ --version
powershell -NoProfile -ExecutionPolicy Bypass `
  -File .\prng_sample\build.ps1 `
  -Test -TestSafe512
```

Lệnh `-Test` chạy các test thông thường và demo nhanh; `-TestSafe512` thêm test sinh safe prime 512 bit. Nếu chỉ cần build, bỏ cả hai cờ. Script dừng khi compiler hoặc test trả lỗi. Cờ execution policy chỉ áp dụng cho tiến trình PowerShell này và không tự cho phép executable bị Windows App Control chặn.

```powershell
.\prng_sample\bin\dh.exe --test
.\prng_sample\bin\dh.exe
```

Trên CMD, đổi thư mục bằng lệnh dưới rồi dùng cùng lệnh gọi PowerShell; chạy executable bằng đường dẫn dùng dấu gạch chéo ngược. Gõ lệnh đọc mã thoát ngay sau chương trình cần kiểm tra.

```bat
cd /d "D:\C++\CryptoLab"
prng_sample\bin\dh.exe --test
echo Exit code: %ERRORLEVEL%
```

## Linux/WSL và Makefile

```sh
cd '/mnt/d/C++/CryptoLab/prng_sample'
make test
./bin/test-prime --safe512
make test-dh
make run-dh
```

Đường dẫn trên dành cho WSL khi source ở ổ D; trên Linux thường cần thay bằng vị trí checkout. Windows có GNU Make có thể dùng `mingw32-make` tại thư mục `prng_sample`. Target `all` chỉ build, `test` build rồi chạy test thường; `clean` xóa đầu ra đã tạo. Cần build lại sau khi sửa source hoặc đổi giữa compiler Windows và Linux.

## Thay đổi tham số và nhận biết kết quả

Chế độ `--test` dùng `p=23` với seed cố định và in cả hai bí mật. Không truyền đối số thì demo lấy seed hệ điều hành, sinh modulus 512 bit và chỉ in tham số, khóa công khai cùng thông báo thành công. Đối số không hợp lệ trả mã `1`. Dòng thành công là:

```text
Diffie-Hellman succeeded: shared secrets match.
```

Kích thước được chọn tại lời gọi `generate_safe_prime(&ctx, 512)` trong main; thư viện hiện hỗ trợ tối đa 512 bit. Muốn tăng trên giới hạn này cần mở rộng kiểu số lớn và kiểm tra lại toàn bộ phép toán, không chỉ đổi tham số. Số vòng mặc định của kiểm tra prime là 40; giới hạn thử cặp trao đổi mặc định là 128. Các trường hợp nhỏ chỉ phục vụ phát triển và không thay thế lần chạy 512 bit khi nghiệm thu.

# Kiểm thử và kết quả

## Phương pháp và phạm vi bằng chứng

Bộ kiểm tra sử dụng `assert`, seed cố định được khởi tạo đầy đủ và các kết quả số học đã biết. Một số test so sánh tính chất toán học; test DH xác minh hai bên tính cùng bí mật. Cách làm này cần kết hợp kiểm tra nền vì hai bên cùng dùng một phép toán sai vẫn có thể cho kết quả giống nhau.

Các kết quả sau được tổng hợp từ ghi nhận trong repo: đợt ngày 08/11/2026 đã chạy các module và DH 512 bit thành công trên Windows/MinGW GCC 6.3.0, C++14; đợt ngày 11/11/2026 bổ sung test biên số lớn và ghi nhận toàn bộ test thông thường qua. Đợt 11/11 không chạy thêm cờ `-TestSafe512`. Các số đo thời gian dưới đây là lần chạy đã ghi nhận, không phải benchmark mới thực hiện khi soạn báo cáo ngày 13/11/2026.

## Danh sách bộ kiểm tra

| Bộ kiểm tra | Nội dung chính | Trạng thái theo log repo |
| --- | --- | --- |
| `test-u512` | Carry/borrow nhiều limb; wrap; bit biên; dịch; hex; nhân/lũy thừa cơ bản. | Qua; có bổ sung ngày 11/11. |
| `test-private-key` | Context rỗng; modulus nhỏ; tái lập; ranh giới limb đến 512 bit; DH nhỏ. | Qua. |
| `test-private-key-extended` | Tần suất mẫu; khóa 512 bit; overload lấy seed OS; so sánh bí mật. | Qua. |
| `test-prng` | Ghi đúng vùng buffer; độ dài không; đối chiếu các hàm lấy số với luồng SHAKE. | Qua. |
| `test-prime` | Prime/hợp số đã biết; Carmichael; phép mô-đun; prime nhỏ; safe prime nhỏ. | Qua. |
| `test-prime --safe512` | Sinh p đủ 512 bit và kiểm tra lại cả p/q. | Qua trong đợt có chạy riêng. |
| `test-regressions` | Dư với modulus 64 bit lớn; chuẩn hóa add/sub; parser sai; byte PRNG. | Qua. |
| `test-dh-exchange` | Khóa bằng q; hết lượt; nguồn sai; bí mật 1; xóa kết quả khi lỗi. | Qua. |
| `dh --test`, `dh` | Demo nhỏ và toàn bộ trao đổi với safe prime 512 bit. | Đã có lần chạy thành công. |

## Kết quả cụ thể có thể đối chiếu

Các ca cộng/trừ kiểm tra việc cộng `1` vào `2^512-1` cho `0`, còn `0-1` cho `2^512-1`. Ca dư lớn kiểm tra `2^64 mod (2^64-1) = 1`. Các phép dịch ở bit `63`, `64`, `511` và độ dịch `512` dùng để phát hiện lỗi ghép limb hoặc truy cập ngoài miền.

Trong test biên DH, với `p=23`, `q=11`, `g=5`, nguồn giả lập trả cặp đầu `(11,2)` khiến khóa công khai bằng `22` và bị loại; cặp `(2,3)` tiếp theo tạo bí mật chung `8`. Bộ kiểm tra cũng xác nhận hai cặp liên tiếp có khóa bằng `q` sẽ hết ngân sách đã cấu hình và không trả lại kết quả cũ.

Với seed chuẩn của chế độ demo nhanh, kết quả được ghi nhận là:

```text
p = 0x17; q = 0xb; g = 0x5
Alice public key = 0x13
Bob public key = 0x8
Alice secret = 0x2
Bob secret = 0x2
Diffie-Hellman succeeded: shared secrets match.
```

Một lần sinh safe prime 512 bit với seed test cố định mất khoảng `0.80` giây trên môi trường đã ghi, gồm kiểm tra lại trong bài test. Đây là giá trị tham khảo của một lượt; số ứng viên và thời gian không cố định. Cặp prime có thể tái hiện bằng `test-prime --safe512` với marker seed trong source. Không dùng tốc độ này để khẳng định kết quả cho máy khác.

## Hạn chế của kiểm thử

Một số lần chạy trước đó bị Windows Application Control chặn executable. Đây là lỗi môi trường; các bản ghi thành công về sau được phân biệt riêng, không coi build thành công là bằng chứng runtime. Linux/WSL, MSVC, nhánh compiler 128 bit và đường lỗi API entropy chưa được chạy xác nhận đầy đủ trong các ghi nhận hiện có.

Mẫu thống kê hữu hạn không chứng minh PRNG an toàn hay phân phối hoàn toàn đều; tính đều của rejection sampling được lý giải bằng mô hình lấy mẫu. Dữ liệu test hiện chưa thay cho kiểm tra chuyên sâu về side-channel, quản lý bộ nhớ bí mật hoặc độ bền trước đầu vào mạng không tin cậy.

# Tự đánh giá và hướng cải thiện

## Ưu điểm

Chương trình bao phủ luồng tính toán chính của đề: biểu diễn số lớn, lũy thừa mô-đun, kiểm tra/sinh safe prime, lấy mẫu khóa và xác minh trao đổi. Tách module giúp đọc code và kiểm tra từng tầng. Vùng trung gian 1024 bit tránh mất phần cao của tích; Montgomery tăng hiệu quả ở modulus lẻ. Các test biên và regression bổ sung cho ví dụ chạy thông thường, trong khi seed xác định giúp tái hiện lỗi.

Các nhánh Windows/Linux được tách tại phần lấy entropy; build chọn rõ C++14 và bật cảnh báo. Các tham số không hợp lệ được trả lỗi hoặc ngoại lệ tùy API, các cặp DH suy biến có ngân sách thử hữu hạn và không che sai khác số học bằng cách lặp lại.

## Hạn chế

Độ rộng dữ liệu cố định giới hạn modulus ở 512 bit. Việc tìm prime chưa có giới hạn thời gian hay khả năng hủy. Chi phí khởi tạo Montgomery vẫn cần được tính đến trong các lời gọi ngắn; một số đường số học còn dùng lấy dư theo bit để ưu tiên tính rõ ràng. API có nhiều cơ chế báo lỗi cần được mô tả thống nhất khi mở rộng ứng dụng.

Chương trình không tính toán theo thời gian hằng, không xóa khóa riêng khỏi bộ nhớ bằng cơ chế bảo đảm và chưa có kiểm tra side-channel. Các tham số và kích thước đáp ứng bài học không đủ để tuyên bố an toàn cho sản phẩm thực tế. Phần trao đổi đang chạy trong một tiến trình, chưa có xác thực chống xen giữa hay KDF. Kết quả kiểm thử hiện tập trung vào Windows và chưa có ma trận CI đa nền tảng.

## Hướng cải thiện

Ưu tiên tiếp theo là chuẩn hóa thông báo lỗi, bổ sung log kiểm thử có phiên bản compiler/seed và tự động chạy trên Windows/Linux. Cần thêm kiểm tra chéo phép toán bằng công cụ độc lập trong môi trường kiểm thử, mở rộng ca biên của Montgomery và xử lý nguồn entropy thất bại. Có thể thiết kế deadline hoặc ngân sách tìm prime để ứng dụng phản hồi rõ hơn.

Nếu phát triển ngoài phạm vi học tập, cần thay đổi hệ thống số lớn để hỗ trợ kích thước phù hợp, lựa chọn nhóm chuẩn hóa, thêm xác thực và dẫn xuất khóa, đồng thời dùng thư viện mật mã đã được đánh giá. Những hướng này không được tính là đã triển khai trong bản hiện tại.

# Tài liệu tham khảo

[1] Ben Lynn. Primality Tests — Miller–Rabin. Stanford PBC Notes. https://crypto.stanford.edu/pbc/notes/numbertheory/millerrabin.html. Truy cập 13/11/2026.

[2] NIST. FIPS 202: SHA-3 Standard: Permutation-Based Hash and Extendable-Output Functions. 2015. https://csrc.nist.gov/pubs/fips/202/final. Truy cập 13/11/2026.

[3] D. Gillmor. RFC 7919: Negotiated Finite Field Diffie-Hellman Ephemeral Parameters for Transport Layer Security (TLS). IETF, 2016. https://www.rfc-editor.org/rfc/rfc7919.html. Truy cập 13/11/2026.

[4] E. Rescorla. RFC 8446: The Transport Layer Security (TLS) Protocol Version 1.3. IETF, 2018. https://www.rfc-editor.org/rfc/rfc8446.html. Truy cập 13/11/2026.

# Bảng đánh giá thành viên

| MSSV | Tên | Những gì đã làm | Tiến độ |
| --- | --- | --- | --- |
| `23120037` | Nguyễn Thanh Khánh Hà | Task 2: Triển khai `generate_safe_prime` để sinh số nguyên tố lớn an toàn. | 100% |
| `23120068` | Lê Nguyễn Minh Nhật | Task 1: Cài đặt `modular_exponentiation` để tính lũy thừa mô-đun hiệu quả. | 100% |
| `23120074` | Nguyễn Văn Phước | Task 4: Hoàn thành trao đổi khóa Diffie–Hellman. | 100% |
| `23120154` | Nguyễn Thanh Phong | Task 3: Triển khai `generate_private_key` để sinh khóa riêng cho Alice và Bob. | 100% |
