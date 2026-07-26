#include "rtos_resources.h"

namespace Resources {

QueueHandle_t logQueue = nullptr;
QueueHandle_t imageQueue = nullptr;
QueueHandle_t telegramQueue = nullptr;

SemaphoreHandle_t captureSemaphore = nullptr;

EventGroupHandle_t systemEvents = nullptr;

void createAll() {
    logQueue = xQueueCreate(LOG_QUEUE_LEN, sizeof(LogMessage));
    imageQueue = xQueueCreate(IMAGE_QUEUE_LEN, sizeof(CameraCaptureResult));
    telegramQueue = xQueueCreate(TELEGRAM_QUEUE_LEN, sizeof(TelegramMessage));

    // Binary semaphore starts "empty" (must be explicitly given before it
    // can be taken) — xSemaphoreCreateBinary() already guarantees this.
    captureSemaphore = xSemaphoreCreateBinary();

    systemEvents = xEventGroupCreate();

    configASSERT(logQueue != nullptr);
    configASSERT(imageQueue != nullptr);
    configASSERT(telegramQueue != nullptr);
    configASSERT(captureSemaphore != nullptr);
    configASSERT(systemEvents != nullptr);
}

} // namespace Resources
