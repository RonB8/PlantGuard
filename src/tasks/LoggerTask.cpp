#include "tasks/LoggerTask.h"

#include <Arduino.h>
#include <cstdarg>
#include <cstdio>
#include "esp_task_wdt.h"

#include "logging.h"
#include "rtos_resources.h"

namespace {

const char* levelTag(LogLevel level) {
    switch (level) {
        case LogLevel::LOG_DEBUG: return "DEBUG";
        case LogLevel::LOG_INFO:  return "INFO ";
        case LogLevel::LOG_WARN:  return "WARN ";
        case LogLevel::LOG_ERROR: return "ERROR";
    }
    return "?????";
}

} // namespace

namespace Logger {

void log(LogLevel level, const char* fmt, ...) {
    if (Resources::logQueue == nullptr) {
        return; // called before Resources::createAll(); nothing we can do
    }

    LogMessage msg;
    msg.level = level;

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg.text, sizeof(msg.text), fmt, args);
    va_end(args);

    // Never block a producer task waiting for log space — logging must be
    // strictly non-blocking. If the queue is full we drop the message
    // rather than risk stalling a sensor or network task.
    xQueueSend(Resources::logQueue, &msg, 0);
}

} // namespace Logger

void LoggerTask(void* pvParameters) {
    (void)pvParameters;
    esp_task_wdt_add(nullptr);

    LogMessage msg;
    for (;;) {
        // Blocks here almost all the time; this is the intended "idle"
        // task and does not spin or poll, so feeding the watchdog only
        // needs to happen once per received message / timeout tick.
        if (xQueueReceive(Resources::logQueue, &msg, pdMS_TO_TICKS(1000)) == pdTRUE) {
            Serial.printf("[%10lu][%s] %s\r\n",
                          static_cast<unsigned long>(millis()),
                          levelTag(msg.level),
                          msg.text);
        }
        esp_task_wdt_reset();
    }
}
