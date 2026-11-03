# Kế hoạch thực hiện Lab01

## 1. Mục tiêu và lịch cam kết

Đồ án: **Số nguyên tố và trao đổi khóa Diffie–Hellman**. Nhóm 4 người hoàn thành mã nguồn, kiểm tra và báo cáo trong một tuần để chủ động thời gian cá nhân.

- Lịch làm chính đề xuất: **03–09/11/2026**, tính 03/11 là ngày 1.
- Mốc hoàn thành nội bộ: **09/11/2026** — có ZIP sẵn sàng nộp.
- Dự phòng: **10–15/11/2026** — chỉ sửa lỗi, bổ sung minh chứng và xử lý sự cố.
- Hạn cuối: **16/11/2026**. Giờ đóng cổng cần kiểm tra trên trang môn học; không mặc định là cuối ngày. Ưu tiên nộp trước 15/11.

Lịch trên là kế hoạch nội bộ, cần cả nhóm xác nhận khả năng tham gia ngay ngày đầu. Nếu bắt đầu muộn, ưu tiên giữ nguyên các đầu ra bắt buộc và điều chỉnh mốc công khai.

## 2. Phạm vi và tiêu chí hoàn thành

Theo Lab01.pdf, bản bắt buộc cần có:

- C/C++, phiên bản C++14 trở xuống; số nguyên tố có độ lớn tối thiểu 512 bit.
- Hệ thống số lớn đủ cho các phép toán của đồ án, có giải thích biểu diễn và xử lý tràn.
- Lũy thừa mô-đun đúng và hiệu quả.
- Tự cài đặt kiểm tra nguyên tố và sinh safe prime: p và q = (p−1)/2 đều nguyên tố theo phép kiểm tra đã chọn. Nếu dùng Miller–Rabin, ghi rõ số vòng và tính xác suất của kết quả.
- Khóa riêng trong [2, p−2], chọn phần tử sinh g có kiểm tra phù hợp.
- Diffie–Hellman chạy được, hai bên tính cùng bí mật chung.
- Báo cáo PDF, hướng dẫn chạy và ZIP đúng cấu trúc đề bài.

Phần điểm cộng về số ngẫu nhiên làm sau yêu cầu bắt buộc. Tối ưu nâng cao, hỗ trợ nhiều nền tảng và giao diện đẹp không phải ưu tiên tuần đầu.

## 3. Phân công và người dự phòng

Thay A/B/C/D bằng tên thành viên khi chốt kế hoạch. Mỗi người chịu trách nhiệm cả code, kiểm tra và nội dung báo cáo của phần mình.

| Người | Phần chính | Báo cáo | Người dự phòng |
| --- | --- | --- | --- |
| A | Kiểu số lớn; cộng, trừ, so sánh, dịch bit; truy cập bit và xuất kết quả | Biểu diễn số lớn, toán tử, giới hạn | B |
| B | Nhân mô-đun, lũy thừa mô-đun; phối hợp xử lý tràn | Thuật toán, độ phức tạp, hiệu năng | A |
| C | Miller–Rabin, lọc ứng viên, sinh safe prime | Kiểm tra nguyên tố, cách sinh p/q | D |
| D | Khóa riêng, chọn g, luồng DH, build và tích hợp | Ứng dụng DH, hướng dẫn chạy; biên tập bản cuối | C |

A–B và C–D kiểm tra chéo. Người dự phòng phải đọc và chạy được phần đã bàn giao, không chỉ nhận tên trên bảng. D điều phối tích hợp nhưng mỗi tác giả tự sửa phần mình. Ai xong sớm hỗ trợ điểm nghẽn thay vì chờ.

## 4. Chuẩn bị trong ngày đầu

- [ ] Đọc Lab01.pdf; thống nhất phạm vi bắt buộc và câu hỏi cần hỏi giảng viên.
- [ ] Điền tên, khung giờ rảnh, khả năng bàn giao từng ngày và người nộp bài.
- [ ] Tạo kho Git dùng chung; đưa mã đã giải nén vào kho. Hiện thư mục chưa có Git metadata.
- [ ] Chọn Linux/WSL làm môi trường chung, có g++ và GNU Make. Chạy mẫu bằng `cd prng_sample` rồi `make run-prng`.
- [ ] Bổ sung cờ chuẩn C++14 vào build; kiểm tra từ một bản checkout sạch.
- [ ] Chốt giao diện hàm, các phép toán số lớn, mã lỗi và quy tắc tràn.
- [ ] Tạo khung báo cáo, thư mục kiểm tra và bảng theo dõi việc.
- [ ] Thống nhất giờ cập nhật ngắn hằng ngày; ghi người chính, dự phòng, hạn và điều kiện nghiệm thu cho từng task.

Lưu ý mã mẫu: PRNG đang dùng sys/random.h/getrandom nên chưa chạy trực tiếp trên Windows; seed[32] phải được khởi tạo đầy đủ. Các header số lớn/DH mới có khai báo; Makefile hiện chỉ có chương trình PRNG.

## 5. Hợp đồng kỹ thuật để làm song song

Chốt một bản giao diện chung trước khi triển khai độc lập:

- Quy định digits[0] là limb thấp hay cao, cách đánh số bit, khởi tạo và xuất số. Khuyến nghị dùng limb thấp trước và ghi rõ trong code.
- Chọn cách nhân mô-đun tránh mất phần cao: trung gian đủ rộng hoặc cộng/nhân mô-đun có kiểm soát tràn. Không cắt tích 1024 bit xuống 512 bit rồi mới lấy dư.
- Chốt hàm hỗ trợ: số bit, chẵn/lẻ, dịch bit, lấy dư, lấy số ngẫu nhiên trong khoảng và các phép so sánh.
- Giữ các giao diện trong dh_sample.h nếu phù hợp; mọi thay đổi chữ ký phải báo cả nhóm trước khi merge.
- C có thể kiểm tra logic nguyên tố bằng số nhỏ; D dựng DH với tham số nhỏ đã biết trong lúc chờ A/B. Sau đó bắt buộc kiểm tra lại bằng số lớn thật.
- Với safe prime p = 2q + 1, nếu chọn phần tử sinh toàn nhóm, kiểm tra g trong khoảng hợp lệ, g² mod p ≠ 1 và g^q mod p ≠ 1; không mặc định g = 2 luôn phù hợp.
- Thư viện PRNG/SHAKE có sẵn được dùng; tự triển khai phần sinh và kiểm tra nguyên tố theo đề. Không dùng thư viện số lớn để thay phần nhóm đã cam kết tự xây dựng.

## 6. Tiến độ 7 ngày và đầu ra

| Ngày | Công việc trọng tâm | Đầu ra nghiệm thu |
| --- | --- | --- |
| 1 — 03/11 | Chuẩn bị môi trường, Git, giao diện; chia việc; mở khung báo cáo | Mọi người chạy được PRNG; giao diện và phân công đã chốt |
| 2 — 04/11 | A/B làm phép toán nền; C kiểm tra nguyên tố với số nhỏ; D dựng luồng DH nhỏ | Kiểm tra nền chạy được; các phần có code và ghi chú bàn giao |
| 3 — 05/11 | Tích hợp lũy thừa trên số lớn; hoàn thiện kiểm tra biên | Lũy thừa qua ca mẫu; từng người có bản nháp báo cáo |
| 4 — 06/11 | Ghép Miller–Rabin/safe prime; bắt đầu đo thời gian ở 512 bit | Luồng sinh p/q tích hợp; có log thời gian và lỗi còn tồn tại |
| 5 — 07/11 | Chạy DH đầy đủ ở 512 bit; ghép báo cáo | Có p, q, g hợp lệ, khóa đúng khoảng, hai bí mật trùng nhau |
| 6 — 08/11 | Kiểm tra chéo, sửa lỗi, hoàn thiện báo cáo và README | Người dự phòng tự build/chạy theo tài liệu; PDF nháp hoàn chỉnh |
| 7 — 09/11 | Build sạch, kiểm tra nguồn và PDF, đóng gói | ZIP sẵn sàng nộp; nhóm xác nhận checklist cuối |

Mốc ngày 4 là cảnh báo sớm: nếu chưa sinh được safe prime 512 bit, cả nhóm ưu tiên tìm điểm nghẽn thay vì tiếp tục phần tùy chọn. Không đợi đến ngày 7 mới chạy toàn bộ chương trình.

## 7. Kế hoạch kiểm tra

Chưa có framework hay target test; nhóm bổ sung bộ kiểm tra nhỏ và ghi rõ lệnh chạy. Không chỉ kiểm tra rằng hai bí mật DH bằng nhau, vì lỗi chung ở phép toán có thể cho kết quả trùng nhưng sai.

- A: 0, 1, giá trị biên; carry/borrow qua nhiều limb; so sánh; dịch qua ranh giới 64 bit; quy tắc tràn đã thống nhất.
- B: số mũ 0/1, modulus 1, đầu vào không hợp lệ theo hợp đồng; ca lũy thừa biết trước; nhân gần giới hạn 512 bit.
- C: số nhỏ, số chẵn, số nguyên tố, hợp số và số Carmichael; đủ số bit; kiểm tra cả p và q. Ghi số vòng Miller–Rabin.
- D: giới hạn khóa riêng; điều kiện của g; ví dụ DH nhỏ tính tay; DH 512 bit với nhiều seed xác định.
- Cả nhóm: ghi seed, kích thước bit, máy chạy, thời gian và kết quả để tái hiện. Seed cố định chỉ dùng cho học tập/kiểm tra.

## 8. Kế hoạch báo cáo

Tạo bản nháp từ ngày 1, viết cùng code và ghép ngày 5. Mỗi phần gồm: mục tiêu, thuật toán, quyết định thiết kế, kiểm tra và hạn chế.

1. Tổng quan bài toán và kiến trúc chương trình — D tổng hợp.
2. Hệ thống số lớn, biểu diễn và toán tử — A.
3. Nhân và lũy thừa mô-đun — B.
4. Kiểm tra nguyên tố và sinh safe prime — C.
5. Trao đổi khóa và một ứng dụng thực tế, có nguồn tham khảo — D.
6. Hướng dẫn build/chạy, thay đổi tham số, ví dụ kết quả — D; tác giả từng module bổ sung.
7. Kết quả kiểm tra, thời gian thực thi, ưu/nhược điểm và cải thiện — cả nhóm.
8. Tính ngẫu nhiên trong mật mã — tùy chọn điểm cộng khi còn thời gian.

Ghi nguồn cho nội dung tham khảo; mô tả đúng bản triển khai cuối. Trước xuất PDF, kiểm tra dấu tiếng Việt, công thức, bảng, số trang và lệnh chạy.

## 9. Quy tắc phối hợp và xử lý chậm tiến độ

- Mỗi ngày cập nhật ba ý: đã xong gì, đang vướng gì, bàn giao gì tiếp theo. Đẩy code đang làm kèm trạng thái rõ ràng.
- Mỗi thay đổi tích hợp phải build được; gửi kèm cách kiểm tra. Giữ một bản chung chạy được sau mỗi mốc.
- Nếu dự kiến trễ, báo trước hạn. Người dự phòng nhận code, ghi chú và ca lỗi để tiếp quản; nhóm chuyển người hỗ trợ nếu ảnh hưởng đường phụ thuộc.
- Nếu A/B chậm cuối ngày 2, D hỗ trợ kiểm tra/build nền; giảm việc biên tập và bỏ phần tùy chọn trước.
- Nếu safe prime chạy lâu, đo riêng lọc ứng viên, Miller–Rabin và nhân mô-đun để tối ưu đúng chỗ. Không giảm xuống dưới 512 bit cho bản nghiệm thu.
- Ngày 10–15/11 chỉ sửa lỗi đã xác định và bổ sung cần thiết; hạn chế đổi kiến trúc khi bản bắt buộc đã đạt.

## 10. Checklist đóng gói và nộp bài

- [ ] Mã nguồn build được ở môi trường đã ghi, chuẩn tối đa C++14.
- [ ] Chạy được quy trình đầy đủ với p ít nhất 512 bit và có minh chứng kiểm tra.
- [ ] README ghi lệnh build/chạy thực tế, tham số, giới hạn và thời gian tham khảo.
- [ ] Báo cáo PDF đầy đủ nội dung bắt buộc; tên/MSSV chính xác.
- [ ] Người khác trong nhóm giải nén ZIP ở thư mục mới và chạy lại thành công.
- [ ] ZIP tên `<MSSV1-MSSV2-MSSV3-MSSV4>.zip`, MSSV theo thứ tự danh sách lớp.
- [ ] Trong ZIP có `project_01_source/` chứa mã nguồn và tài nguyên build cần thiết; `project_01_report/` chứa báo cáo PDF.
- [ ] Người được phân công nộp trước hạn, lưu xác nhận; một người khác kiểm tra lại trạng thái nộp.

Nguồn yêu cầu: Lab01.pdf trong thư mục dự án. Kế hoạch này là tài liệu điều phối, không thay thế báo cáo đồ án.
