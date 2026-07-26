// ============================================================================
// main.cpp — Smart Plant Guardian entry point.
//
// setup() does nothing but bring up shared infrastructure (logging queue,
// WiFi, watchdog) and spawn the five worker tasks described in the task
// table below; all real behavior lives in src/tasks/*.cpp. loop() is left
// as an idle heartbeat since the Arduino default task isn't used for
// anything (see the rationale in the comment above it).
//
// Task table
// ----------
//  Task           Prio  Stack   Core  Responsibility
//  LoggerTask      1    3072    any   Drains logQueue, owns Serial
//  SensorTask      2    4096    any   Soil moisture (10s) + VL53L0X (100ms)
//  CameraTask      3    8192    1     Blocks on captureSemaphore, shoots JPEG
//  AITask          2    6144    any   Uploads JPEG, applies dog/confidence rule
//  TelegramTask    2   12288    any   Sends text/photo alerts over HTTPS
//
// CameraTask is pinned to core 1 (APP_CPU) and given the highest priority
// of the five so a capture is not delayed behind WiFi/BT housekeeping that
// the core-0 protocol stack runs; everything else is left unpinned since
// none of it is latency-critical relative to network round-trip time.
// TelegramTask gets the largest stack because WiFiClientSecure's TLS
// handshake is the most stack-hungry operation in this firmware.
// ============================================================================

#include <Arduino.h>
#include "esp_task_wdt.h"

#include "config.h"
#include "logging.h"
#include "rtos_resources.h"
#include "wifi.h"

#include "tasks/AITask.h"
#include "tasks/CameraTask.h"
#include "tasks/LoggerTask.h"
#include "tasks/SensorTask.h"
#include "tasks/TelegramTask.h"

namespace {

void createTasks() {
    xTaskCreate(LoggerTask, "LoggerTask", 3072, nullptr, 1, nullptr);
    xTaskCreate(SensorTask, "SensorTask", 4096, nullptr, 2, nullptr);
    xTaskCreatePinnedToCore(CameraTask, "CameraTask", 8192, nullptr, 3, nullptr, 1);
    xTaskCreate(AITask, "AITask", 6144, nullptr, 2, nullptr);
    xTaskCreate(TelegramTask, "TelegramTask", 12288, nullptr, 2, nullptr);
}

} // namespace

void setup() {
    Serial.begin(115200);

    Resources::createAll();
    // LoggerTask isn't running yet, but logQueue already exists, so early
    // Logger::log() calls below simply buffer until the task starts.
    createTasks();

    esp_task_wdt_init(Config::TASK_WDT_TIMEOUT_S, true /* panic on timeout */);

    LOG_INFO("Smart Plant Guardian booting...");

    WiFiManager::begin();

    LOG_INFO("Setup complete, %u tasks running", 5);
}

void loop() {
    // Arduino's default loop task is otherwise idle; reuse it as the WiFi
    // reconnect poller so no extra task/timer is needed for that concern.
    WiFiManager::poll();
    vTaskDelay(pdMS_TO_TICKS(1000));
}
