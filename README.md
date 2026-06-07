# mangcambien-cuoiky-nguyenthietchan
Yoga Pose Detection using Edge Impulse and ESP32-CAM
Giới thiệu
Thông tin sinh viên:
Nguyễn Thiết Chân 
N23DCCI008 
D23CQCI01-N PTITHCM 
Gmail sv: n23dcci008@student.ptithcm.edu.vn
Gmail classroom: dragonsugar740@gmail.com

Tên đề tài : Nhận dạng pose yoga cơ bản (cây, chiến binh, tam giác, núi)
Sử dụng ESP32-Cam AI Thinker & Egde Impluse (FOMO)
Sau đó so sánh với Mediapipe

Các tư thế được nhận dạng:

Mountain
Tree
Warrior
Triangle
Công nghệ sử dụng
Edge Impulse
FOMO Object Detection
ESP32-CAM AI Thinker + ESP32- CAM MB
Arduino IDE
## Cấu trúc thư mục

```text
mangcambien-cuoiky-nguyenthietchan/

├── README.md
├── esp32cam/
│   ├── yoga_pose_detection.ino
│
├── model/
│   └── edge-impulse-sdk/
│
├── screenshots/
│   ├── Create_imlpuse.jpg
│   ├── Dashboard.jpg
│   ├── Data_acquistion.jpg
│   ├── Data_Distribution.jpg
│   ├── BoundingBox.jpg
│   ├── feature_explorer.jpg
│   ├── Deployment.jpg
│   ├── confusion_matrix.png
│   ├── Thresholds.jpg
│   └── Ketqua.jpg
│
├── report/
│   ├── NguyenThietChan_final_cuoiky.docx
│   └── NguyenThietChan_slide_cuoiky.pptx
│
└── dataset/
    └── sample_images/
```

---

## Phần cứng sử dụng

- ESP32-CAM AI Thinker
- ESP32-CAM-MB
- Camera OV2640
- Cáp USB

---

## Công nghệ sử dụng

- Edge Impulse Studio
- FOMO Object Detection
- MobileNetV2
- Arduino IDE

---

## Hướng dẫn cài đặt

### Bước 1

Cài đặt Arduino IDE.

### Bước 2

Cài đặt ESP32 Board Package.

### Bước 3

Tải thư viện Edge Impulse đã export từ dự án.

### Bước 4

Mở file:

```text
esp32cam/yoga_pose_detection.ino
```

### Bước 5

Chọn board:

```text
AI Thinker ESP32-CAM
```

### Bước 6

Kết nối ESP32-CAM-MB với máy tính bằng cáp USB.

### Bước 7

Nạp chương trình.

### Bước 8

Mở Serial Monitor để theo dõi kết quả nhận dạng.

---

## Kết quả đạt được

Mô hình nhận dạng được 4 tư thế Yoga cơ bản:

- Mountain
- Tree
- Warrior
- Triangle

Kết quả huấn luyện:

| Class | Accuracy |
|---------|---------|
| Mountain | 54.5% |
| Warrior | 55.6% |
| Triangle | 37.5% |
| Tree | 81.0% |

F1-score tổng thể: 59.%

---

## Link dự án Edge Impulse

https://studio.edgeimpulse.com/public/1021172/live

---

## Tài liệu tham khảo

1. Edge Impulse Documentation
2. ESP32-CAM Documentation
3. MediaPipe Pose Documentation
4. TinyML - O'Reilly Media
5. Chat GPT
6. Gemini
7. 
