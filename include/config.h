#pragma once

// ============================================================================
// config.h — Central, single-source-of-truth configuration.
//
// Nothing in this file touches hardware. It only defines constants that the
// rest of the firmware reads. Keep secrets (WiFi password, bot token) here
// for simplicity, matching the project brief; in a real product these would
// come from NVS / a provisioning flow instead of being compiled into the
// binary.
// ============================================================================

#include <cstdint>

namespace Config {

// ---------------------------------------------------------------------------
// WiFi
// ---------------------------------------------------------------------------
constexpr char WIFI_SSID[]     = "YOUR_WIFI_SSID";
constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";

constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS     = 15000; // first-boot connect attempt
constexpr uint32_t WIFI_RECONNECT_RETRY_MS     = 5000;  // gap between reconnect attempts
constexpr uint32_t WIFI_RECONNECT_MAX_BACKOFF_MS = 60000;

// ---------------------------------------------------------------------------
// Telegram Bot API
// ---------------------------------------------------------------------------
constexpr char TELEGRAM_BOT_TOKEN[] = "123456789:REPLACE_WITH_YOUR_BOT_TOKEN";
constexpr char TELEGRAM_CHAT_ID[]   = "REPLACE_WITH_YOUR_CHAT_ID";

constexpr uint32_t TELEGRAM_HTTP_TIMEOUT_MS = 10000;
constexpr uint8_t  TELEGRAM_MAX_RETRIES     = 3;
constexpr uint32_t TELEGRAM_RETRY_BACKOFF_MS = 2000;

// ---------------------------------------------------------------------------
// AI inference server (external — not implemented by this firmware)
// ---------------------------------------------------------------------------
constexpr char AI_SERVER_URL[] = "http://192.168.1.100:5000/predict";

constexpr uint32_t AI_HTTP_TIMEOUT_MS    = 15000;
constexpr uint8_t  AI_MAX_RETRIES        = 2;
constexpr uint32_t AI_RETRY_BACKOFF_MS   = 1500;

constexpr char  AI_TARGET_OBJECT[]            = "dog";
constexpr float AI_CONFIDENCE_THRESHOLD       = 0.9f;

// ---------------------------------------------------------------------------
// Soil moisture (Capacitive Soil Moisture Sensor v2.0, analog output)
// ---------------------------------------------------------------------------
constexpr uint8_t  SOIL_MOISTURE_ADC_PIN = 33; // ADC1_CH5 — safe to read with WiFi active

constexpr uint32_t SOIL_READ_INTERVAL_MS         = 10000; // spec: every 10 seconds
constexpr float    SOIL_MOISTURE_THRESHOLD_PERCENT = 35.0f; // configurable threshold

// Raw ADC calibration points. Sensor reads HIGH raw value in dry air and a
// LOW raw value fully submerged in water — the relationship is inverted
// relative to "moisture". Recalibrate for your specific sensor unit.
constexpr int SOIL_ADC_RAW_DRY = 3000; // reading in dry air
constexpr int SOIL_ADC_RAW_WET = 1200; // reading in water

// Basic plausibility bounds used to detect a disconnected/faulty sensor.
constexpr int SOIL_ADC_RAW_MIN_VALID = 200;
constexpr int SOIL_ADC_RAW_MAX_VALID = 4095;

// ---------------------------------------------------------------------------
// Distance sensor (VL53L0X) / dog-detection trigger
// ---------------------------------------------------------------------------
constexpr uint8_t  I2C_SDA_PIN = 13; // dedicated bus, separate from camera SCCB
constexpr uint8_t  I2C_SCL_PIN = 14;

constexpr uint32_t DISTANCE_POLL_INTERVAL_MS = 100;  // sensor task tick
constexpr uint16_t DISTANCE_THRESHOLD_MM     = 500;  // 50 cm, configurable
constexpr uint32_t DISTANCE_SUSTAIN_MS       = 1000; // must stay closer than threshold this long
constexpr uint32_t DOG_DETECTION_COOLDOWN_MS = 30000; // suppress re-trigger while animal lingers

constexpr uint8_t  DISTANCE_SENSOR_MAX_INIT_RETRIES = 5;
constexpr uint32_t DISTANCE_SENSOR_INIT_RETRY_MS    = 2000;

// ---------------------------------------------------------------------------
// Camera (OV2640)
// ---------------------------------------------------------------------------
// framesize/quality kept conservative because this board has no PSRAM
// (see README "Design Decisions"). Raise FRAMESIZE if using a PSRAM part.
constexpr uint32_t CAMERA_XCLK_FREQ_HZ = 20000000;

// ---------------------------------------------------------------------------
// Watchdog
// ---------------------------------------------------------------------------
constexpr uint32_t TASK_WDT_TIMEOUT_S = 15;

// ---------------------------------------------------------------------------
// Misc
// ---------------------------------------------------------------------------
constexpr uint8_t STATUS_LED_PIN = 2;

} // namespace Config
