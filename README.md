RETOFISH Firmware 

Mã nguồn điều khiển Robot cho cá ăn RETOFISH – định hướng giáo dục STEM. Người dùng có thể học, tùy biến (mod) và đóng góp lại cho cộng đồng.

License: CC BY-NC-SA 4.0 — cho phép dùng/học/chia sẻ phi thương mại. Mọi hoạt động thương mại cần sự đồng ý bằng văn bản của RETOBOTS (xem mục License bên dưới).

Tính năng chính

F01 – Cho ăn theo lịch: Tối đa 3 mốc giờ/ngày, lưu EEPROM/flash.
F02 – Điều khiển thủ công: Nhấn đúp (≤1s) khi màn hình đang sáng để cho ăn ngay với liều mặc định. Trong lúc đang cho ăn, lệnh mới sẽ được bỏ qua.
F03 – Đèn trạng thái: LED RGB tắt ở chế độ chờ (tiết kiệm pin); xanh lá khi đang cho ăn.
F04 – Hiển thị trạng thái: TFT SPI 1.47" hiển thị trạng thái & thời gian; nhấn nhanh để bật, tự tắt sau 15s.
F05 – Cấu hình bằng Rotary Encoder: Nhấn giữ >3s để vào SETUP, chỉnh giờ hệ thống, 3 mốc Feeding (giờ/phút/số vòng/Enable). 
F06 - Cho ăn không giới hạn liều lượng bằng cách nhấn nút >6s, dừng khi buông nút

Phần cứng tham chiếu

- MCU: ESP32 (NodeMCU-32S)
- Động cơ: 28BYJ-48 + driver ULN2003 (hoặc thay bằng stepper driver khác nếu bạn mod)
- RTC: DS1307 (I²C)
- Màn hình: TFT SPI 1.47" (dòng ST77xx – dùng với Adafruit GFX/Adafruit ST77xx)
- Nút/Encoder: Rotary Encoder (kèm nút nhấn)
- LED trạng thái: LED RGB

Cài đặt & Build (PlatformIO)

Yêu cầu
- VS Code + PlatformIO IDE
- Driver USB-Serial cho board ESP32 của bạn

Đóng góp (Contributing)

- Mọi đóng góp được cấp phép lại theo CC BY-NC-SA 4.0.
- Quy trình đề xuất:
- Mở Issue mô tả bug/feature.
- Tạo nhánh feat/... hoặc fix/....
- Conventional Commits (feat:, fix:, docs:…).
- Mở Pull Request kèm mô tả & hình/video test.
- Vui lòng xem thêm: CONTRIBUTING.md

License & Thương hiệu

- License: Creative Commons Attribution–NonCommercial–ShareAlike 4.0 (CC BY-NC-SA 4.0).
- Không cho phép sử dụng thương mại khi chưa có đồng ý bằng văn bản của RETOBOTS.
- SPDX: CC-BY-NC-SA-4.0

Thương hiệu: Tên & logo RETOBOTS/RETOFISH là tài sản của RETOBOTS™. Vui lòng xem TRADEMARKS.md.

Liên hệ hợp tác thương mại: retobots.contact@gmail.com

Sứ mệnh của RETOBOTS: mang robotics đến với học sinh sinh viên và người đam mê Robotic, giúp các bạn học – làm – sáng tạo qua dự án thực tế. Cảm ơn bạn đã cùng đóng góp!