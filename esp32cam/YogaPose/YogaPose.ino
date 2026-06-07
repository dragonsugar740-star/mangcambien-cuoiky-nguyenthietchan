#include <Yoga_Detection_PTITHCM_MCB_inferencing.h>

#include <esp_camera.h>
#include "img_converters.h" // Thư viện hỗ trợ chuyển đổi định dạng ảnh

#include <Yoga_Detection_PTITHCM_MCB_inferencing.h>

// Định nghĩa độ phân giải thực tế của Camera quét
#define CAM_WIDTH   320
#define CAM_HEIGHT  240

// Cấu hình chân Pinout chuẩn cho ESP32-CAM AI Thinker
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Khai báo mảng bộ đệm toàn cục để tránh lỗi giải phóng RAM cục bộ
uint8_t *camera_snapshot_buf = NULL;

static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_QVGA, // Sửa lỗi sang QVGA (320x240) tương thích 100% mọi driver
    .jpeg_quality = 12,
    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY
};

bool ei_camera_init(void);
bool ei_camera_capture(uint8_t *out_buf);

// Hàm trích xuất dữ liệu ảnh và tự động căn tỷ lệ (Resize) về kích thước FOMO yêu cầu
int get_camera_data(size_t offset, size_t length, float *out_ptr) {
    size_t out_idx = 0;
    while (out_idx < length) {
        size_t current_offset = offset + out_idx;
        
        // Xác định tọa độ pixel trên lưới mô hình (ví dụ 160x160)
        int model_x = current_offset % EI_CLASSIFIER_INPUT_WIDTH;
        int model_y = current_offset / EI_CLASSIFIER_INPUT_WIDTH;
        
        // Thuật toán lân cận gần nhất (Nearest-neighbor) ánh xạ ngược về ảnh gốc 320x240
        int cam_x = (model_x * CAM_WIDTH) / EI_CLASSIFIER_INPUT_WIDTH;
        int cam_y = (model_y * CAM_HEIGHT) / EI_CLASSIFIER_INPUT_HEIGHT;
        
        size_t cam_pixel_idx = (cam_y * CAM_WIDTH + cam_x) * 3;
        
        uint8_t r = camera_snapshot_buf[cam_pixel_idx + 0];
        uint8_t g = camera_snapshot_buf[cam_pixel_idx + 1];
        uint8_t b = camera_snapshot_buf[cam_pixel_idx + 2];
        
        // Đóng gói định dạng RGB888 thành kiểu dữ liệu float cho Edge Impulse
        out_ptr[out_idx] = (r << 16) | (g << 8) | b;
        out_idx++;
    }
    return 0;
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Hệ thống nhận dạng Yoga Pose - ESP32-CAM...");

    // Khởi tạo Camera phần cứng
    if (!ei_camera_init()) {
        Serial.println("LỖI: Khởi tạo Camera thất bại!");
        while(1);
    }

    // Cấp phát bộ nhớ tĩnh một lần duy nhất cho bộ đệm ảnh trong PSRAM nhằm tăng tốc xử lý
    if (psramFound()) {
        camera_snapshot_buf = (uint8_t *)ps_malloc(CAM_WIDTH * CAM_HEIGHT * 3);
    } else {
        camera_snapshot_buf = (uint8_t *)malloc(CAM_WIDTH * CAM_HEIGHT * 3);
    }

    if (camera_snapshot_buf == NULL) {
        Serial.println("LỖI: Không đủ bộ nhớ RAM để cấp phát bộ đệm ảnh!");
        while(1);
    }
    Serial.println("Hệ thống và bộ đệm ảnh đã sẵn sàng!");
}

void loop() {
    Serial.println("\n--- Đang quét tư thế Yoga ---");

    if (!ei_camera_capture(camera_snapshot_buf)) {
        Serial.println("LỖI: Không thể chụp ảnh từ Camera!");
        delay(1000);
        return;
    }

    // Gán hàm callback xử lý dữ liệu chuẩn C cho Edge Impulse
    signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &get_camera_data;

    // Chạy xử lý mô hình AI (Inference)
    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
    if (res != EI_IMPULSE_OK) {
        Serial.printf("LỖI: Chạy mô hình thất bại (Mã lỗi: %d)\n", res);
        return;
    }

    // Đọc kết quả đầu ra
    bool found_object = false;
    for (size_t i = 0; i < result.bounding_boxes_count; i++) {
        auto bb = result.bounding_boxes[i];
        
        // Áp dụng bộ lọc ngưỡng tin cậy 0.35 đã tối ưu
        if (bb.value >= 0.45) { 
            found_object = true;
            Serial.printf("  👉 [PHÁT HIỆN]: %s | Độ tin cậy: %.2f | Tâm: (x:%d, y:%d)\n", 
                          bb.label, bb.value, bb.x + (bb.width/2), bb.y + (bb.height/2));
        }
    }

    if (!found_object) {
        Serial.println("  ❌ Trạng thái: BACKGROUND (Không nhận diện rõ tư thế)");
    }

    delay(1500); // Tạm dừng 1.5 giây giữa các lượt quét để Serial Monitor dễ đọc
}

bool ei_camera_init(void) {
    if (psramFound() == false) {
        return false;
    }
    
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        return false;
    }

    sensor_t * s = esp_camera_sensor_get();
    s->set_vflip(s, 1);   // Lật ngược ảnh lại nếu camera của bạn bị lộn ngược trần
    s->set_hmirror(s, 0); 
    return true;
}

bool ei_camera_capture(uint8_t *out_buf) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        return false;
    }

    // Giải nén gói tin JPEG từ cảm biến sang mảng màu RGB888 thô
    bool res = fmt2rgb888(fb->buf, fb->len, fb->format, out_buf);
    esp_camera_fb_return(fb);
    return res;
}