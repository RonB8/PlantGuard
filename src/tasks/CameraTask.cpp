#include "tasks/CameraTask.h"

#include <Arduino.h>
#include "esp_task_wdt.h"

#include "camera.h"
#include "config.h"
#include "logging.h"
#include "rtos_resources.h"

namespace {

constexpr uint8_t CAPTURE_MAX_ATTEMPTS = 2; // one retry on transient timeout

bool captureWithRetry(uint8_t** buffer, size_t* length) {
    for (uint8_t attempt = 1; attempt <= CAPTURE_MAX_ATTEMPTS; ++attempt) {
        if (CameraManager::capture(buffer, length)) {
            return true;
        }
        if (attempt < CAPTURE_MAX_ATTEMPTS) {
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
    return false;
}

} // namespace

void CameraTask(void* pvParameters) {
    (void)pvParameters;
    esp_task_wdt_add(nullptr);

    bool cameraReady = CameraManager::init();
    if (!cameraReady) {
        LOG_ERROR("Camera failed to initialize at boot; will retry before first capture");
    }

    for (;;) {
        // Bounded wait (not portMAX_DELAY) purely so the watchdog gets fed
        // periodically even during long idle stretches with no dog nearby.
        if (xSemaphoreTake(Resources::captureSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
            if (!cameraReady) {
                cameraReady = CameraManager::init();
            }

            if (!cameraReady) {
                LOG_ERROR("Capture request dropped: camera not initialized");
            } else {
                LOG_INFO("Capturing image...");
                uint8_t* buffer = nullptr;
                size_t length = 0;

                if (captureWithRetry(&buffer, &length)) {
                    CameraCaptureResult result{buffer, length};
                    if (xQueueSend(Resources::imageQueue, &result, pdMS_TO_TICKS(500)) != pdTRUE) {
                        LOG_ERROR("Image queue full, dropping captured frame");
                        free(buffer);
                    }
                } else {
                    LOG_ERROR("Camera capture failed after retry, skipping this trigger");
                }
            }
        }

        esp_task_wdt_reset();
    }
}
