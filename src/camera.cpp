#include "camera.h"

#include <Arduino.h>
#include <cstring>
#include "esp_camera.h"

#include "config.h"
#include "logging.h"

namespace {

// --- OV2640 <-> ESP32 DevKit V1 pin mapping (see camera.h header comment) --
constexpr int PWDN_GPIO_NUM  = 32;
constexpr int RESET_GPIO_NUM = -1; // not wired; tie sensor RESET to VCC
constexpr int XCLK_GPIO_NUM  = 0;
constexpr int SIOD_GPIO_NUM  = 26;
constexpr int SIOC_GPIO_NUM  = 27;

constexpr int Y9_GPIO_NUM = 35;
constexpr int Y8_GPIO_NUM = 34;
constexpr int Y7_GPIO_NUM = 39;
constexpr int Y6_GPIO_NUM = 36;
constexpr int Y5_GPIO_NUM = 21;
constexpr int Y4_GPIO_NUM = 19;
constexpr int Y3_GPIO_NUM = 18;
constexpr int Y2_GPIO_NUM = 5;
constexpr int VSYNC_GPIO_NUM = 25;
constexpr int HREF_GPIO_NUM  = 23;
constexpr int PCLK_GPIO_NUM  = 22;

bool s_initialized = false;

} // namespace

bool CameraManager::init() {
    camera_config_t config = {};
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    // NOTE: esp32-camera renamed these from pin_sscb_sda/pin_sscb_scl to
    // pin_sccb_sda/pin_sccb_scl (fixing the "SSCB"->"SCCB" typo). If your
    // pinned platform version predates that rename and this fails to
    // compile, swap to the pin_sscb_* spelling.
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = Config::CAMERA_XCLK_FREQ_HZ;
    config.pixel_format = PIXFORMAT_JPEG;

    // No PSRAM on this board (see README "Design Decisions"): keep the
    // frame buffer modest and single-buffered so it fits in internal DRAM.
    config.frame_size = FRAMESIZE_VGA; // 640x480
    config.jpeg_quality = 12;          // lower = better quality, larger file
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        LOG_ERROR("Camera init failed, esp_err=0x%x", err);
        s_initialized = false;
        return false;
    }

    s_initialized = true;
    LOG_INFO("Camera initialized (VGA/JPEG)");
    return true;
}

bool CameraManager::isInitialized() {
    return s_initialized;
}

bool CameraManager::capture(uint8_t** outBuffer, size_t* outLength) {
    if (!s_initialized) {
        LOG_ERROR("Camera capture requested before successful init");
        return false;
    }

    camera_fb_t* fb = esp_camera_fb_get();
    if (fb == nullptr) {
        LOG_ERROR("Camera capture timed out (esp_camera_fb_get returned null)");
        return false;
    }

    uint8_t* copy = static_cast<uint8_t*>(malloc(fb->len));
    if (copy == nullptr) {
        LOG_ERROR("Camera capture: out of memory copying %u-byte frame",
                   static_cast<unsigned>(fb->len));
        esp_camera_fb_return(fb);
        return false;
    }

    memcpy(copy, fb->buf, fb->len);
    size_t len = fb->len;
    esp_camera_fb_return(fb); // release the driver's internal buffer ASAP

    *outBuffer = copy;
    *outLength = len;
    return true;
}
