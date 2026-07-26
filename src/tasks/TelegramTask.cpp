#include "tasks/TelegramTask.h"

#include <Arduino.h>
#include "esp_task_wdt.h"

#include "logging.h"
#include "rtos_resources.h"
#include "telegram.h"

void TelegramTask(void* pvParameters) {
    (void)pvParameters;
    esp_task_wdt_add(nullptr);

    TelegramMessage msg;
    for (;;) {
        if (xQueueReceive(Resources::telegramQueue, &msg, pdMS_TO_TICKS(1000)) == pdTRUE) {
            if ((xEventGroupGetBits(Resources::systemEvents) & Resources::WIFI_CONNECTED_BIT) == 0) {
                LOG_WARN("Telegram task: WiFi down, dropping message");
            } else {
                bool ok = (msg.imageBuffer != nullptr)
                              ? TelegramClient::sendPhoto(msg.text, msg.imageBuffer, msg.imageLength)
                              : TelegramClient::sendMessage(msg.text);

                if (ok) {
                    LOG_INFO("Telegram sent");
                } else {
                    LOG_ERROR("Telegram send failed after retries");
                }
            }

            if (msg.imageBuffer != nullptr) {
                free(msg.imageBuffer);
            }
        }

        esp_task_wdt_reset();
    }
}
