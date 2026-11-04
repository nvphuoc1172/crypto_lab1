## Lab01: Số nguyên tố & Trao đổi khóa Diffie-Hellman

Repo cung cấp thư viện sinh số giả ngẫu nhiên (PRNG) dùng SHAKE128, một XOF
(Extendable-Output Function).

```
make run-prng
```

Dùng các hàm trong thư viện để tự xây dựng số lớn, 512 bit, 1024 bit, etc. (uint512_t, uint1024_t uint2048_t).

### Sinh và kiểm tra số nguyên tố

`src/prime_sample.cpp` cung cấp:

- `is_probable_prime(n, &ctx, rounds)`: lọc chia thử bằng các số nguyên tố nhỏ,
  sau đó kiểm tra Miller–Rabin. Mặc định 40 vòng; với cơ sở ngẫu nhiên độc lập,
  xác suất chấp nhận nhầm một hợp số không quá `4^-40`.
- `generate_prime(&ctx, bitsize, rounds)`: sinh số nguyên tố probable-prime có
  đúng số bit yêu cầu (2 đến 512).
- `generate_safe_prime(&ctx, bitsize)`: tìm `p = 2q + 1`, kiểm tra cả `q` và
  `p` bằng Miller–Rabin 40 vòng; hỗ trợ 3 đến 512 bit.

Khởi tạo `prng_ctx` bằng `prng_seed` trước khi gọi các hàm sinh. Với kiểm thử,
dùng seed cố định; seed đó không phù hợp để bảo vệ khóa thật. Giá trị trả về 0
biểu thị tham số đầu vào không hợp lệ. Mô-đun có thể biên dịch độc lập bằng
`make prime-module`; khi tích hợp chương trình, liên kết thêm `prime_sample.cpp`,
`dh_sample.cpp`, `u512_sample.cpp`, `prng.cpp` và `fips202.c`.

Nhân modulo cho modulus lẻ dùng Montgomery reduction để tăng tốc Miller–Rabin.
Đường nhân 512x512 cũng có bản schoolbook limb 32 bit cho compiler không hỗ trợ
`__int128`.
