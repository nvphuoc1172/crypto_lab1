# Trạng thái triển khai và kiểm thử — 08/11/2026

Tài liệu này đồng bộ trạng thái hiện tại; các log lỗi trong REPO_REVIEW là lịch sử,
không phải danh sách lỗi còn tồn tại. Đây là ghi chú kỹ thuật, không thay thế báo cáo PDF.

## Các module

| Module | Trách nhiệm |
| --- | --- |
| u512_sample.cpp | Số lớn 512/1024 bit, Montgomery, lấy dư và parse hex |
| dh_sample.cpp | Lũy thừa mô-đun |
| prime_sample.cpp | Miller–Rabin, sinh prime và safe prime |
| private_key.cpp | Cả hai overload sinh khóa riêng; p>=5, kết quả [2,p−2] |
| dh_exchange.cpp | Tạo cặp khóa, tính bí mật và xử lý giá trị suy biến |
| main_dh.cpp | Seed hệ điều hành/test, xác minh tham số, chọn g và hiển thị kết quả |

## Hành vi demo

`dh.exe --test` dùng p=23 và seed cố định, in hai bí mật để đối chiếu.
`dh.exe` sinh safe prime 512 bit, kiểm tra p/q, chọn phần tử sinh toàn nhóm,
không in khóa riêng hoặc giá trị bí mật. Hai chế độ đều kiểm tra kết quả bằng nhau.

Khóa riêng bằng q vẫn thuộc miền của hàm sinh nhưng tạo khóa công khai p−1.
Demo loại cặp này và lấy mẫu lại. Nó cũng loại bí mật chung 0, 1, p−1. Tối đa
128 cặp; hết lượt, đầu vào/nguồn khóa không hợp lệ hoặc bí mật hai bên khác nhau
thì trả thất bại. Không retry khi có sai khác số học, tránh che lỗi thuật toán.

Hàm nguồn khóa được truyền vào test để ép tình huống khó gặp. `test_dh_exchange.cpp`
kiểm tra a=q, b=q, bí mật chung 1 trong nhóm tổng hợp cho test, hết lượt thử,
con trỏ rỗng và nguồn khóa trả 0. Nhóm không-safe-prime chỉ dùng để ép nhánh bảo vệ;
main luôn kiểm tra safe prime trước khi trao đổi.

## Kiểm chứng

Đợt đối chiếu trước sửa biên đã chạy thành công trên Windows/MinGW GCC 6.3.0,
C++14: số lớn, private key chính/mở rộng, PRNG, nguyên tố, regression, safe prime
512 bit và demo DH mặc định. Các ghi nhận App Control chặn thuộc lần chạy cũ.

Bản sau sửa đã build/chạy thành công toàn bộ test bằng PowerShell, gồm test biên
mới, safe prime 512 bit và demo DH 512 bit mặc định; exit code 0. Không có cảnh báo
compiler trong lần chạy này. Lệnh tái hiện:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File prng_sample/build.ps1 -Test -TestSafe512
.\prng_sample\bin\dh.exe
```

Từ prng_sample/, có thể dùng `mingw32-make test` trên Windows hoặc `make test`
trên Linux. Bộ test biên mới được tích hợp vào cả hai cách build.
Linux/MSVC và lỗi API entropy chưa được chạy ép lỗi xác nhận.

Chưa hoàn thiện báo cáo Word/PDF và ZIP bài nộp; các artifact này để thực hiện sau.
