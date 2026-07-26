#include "tasks/AITask.h"

#include <Arduino.h>
#include "esp_task_wdt.h"

#include "ai_client.h"
#include "config.h"
#include "detection_logic.h"
#include "logging.h"
#include "rtos_resources.h"

void AITask(void* pvParameters) {
    (void)pvParameters;
    esp_task_wdt_add(nullptr);

    CameraCaptureResult capture;
    for (;;) {
        if (xQueueReceive(Resources::imageQueue, &capture, pdMS_TO_TICKS(1000)) == pdTRUE) {
            bool consumedByTelegram = false;

            if ((xEventGroupGetBits(Resources::systemEvents) & Resources::WIFI_CONNECTED_BIT) == 0) {
                LOG_WARN("AI task: WiFi down, dropping captured frame");
            } else {
                LOG_INFO("Uploading image...");
                AIClassificationResult result;
                if (AIClient::classify(capture.buffer, capture.length, result)) {
                    LOG_INFO("AI result: object=%s confidence=%.2f", result.object, result.confidence);

                    if (DetectionLogic::isDogDetected(result.object, result.confidence,
                                                       Config::AI_TARGET_OBJECT,
                                                       Config::AI_CONFIDENCE_THRESHOLD)) {
                        LOG_INFO("Dog detected");
                        TelegramMessage msg = {};
                        snprintf(msg.text, sizeof(msg.text), "\xF0\x9F\x90\xB6 Dog detected near the plant.");
                        msg.imageBuffer = capture.buffer;
                        msg.imageLength = capture.length;

                        if (xQueueSend(Resources::telegramQueue, &msg, pdMS_TO_TICKS(500)) == pdTRUE) {
                            consumedByTelegram = true; // TelegramTask now owns the buffer
                        } else {
                            LOG_WARN("Telegram queue full, dropping dog-detection alert");
                        }
                    } else {
                        LOG_INFO("Object ignored (not a confident dog match)");
                    }
                } else {
                    LOG_ERROR("AI classification failed, discarding frame");
                }
            }

            if (!consumedByTelegram) {
                free(capture.buffer);
            }
        }

        esp_task_wdt_reset();
    }
}
