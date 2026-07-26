#pragma once

// ============================================================================
// types.h — Plain-data message types that travel through FreeRTOS queues.
//
// Kept free of Arduino/ESP-IDF headers (only <cstdint>/<cstddef>) so that
// pure-logic modules (moisture_utils, detection_logic) and their native unit
// tests can include it without pulling in the whole framework.
//
// Ownership convention: any struct carrying a heap `buffer`/`imageBuffer`
// pointer transfers ownership to whichever task receives it from a queue.
// That task is responsible for freeing the buffer (heap_caps/free) once it
// is done with it, whether it forwards, consumes, or drops the message.
// ============================================================================

#include <cstddef>
#include <cstdint>

enum class LogLevel : uint8_t { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR };

struct LogMessage {
    LogLevel level;
    char text[128];
};

// Produced by CameraTask, consumed by AITask.
struct CameraCaptureResult {
    uint8_t* buffer;
    size_t length;
};

// Produced by AITask (or dropped there), consumed by TelegramTask.
struct TelegramMessage {
    char text[256];
    uint8_t* imageBuffer; // nullptr if this message has no photo attached
    size_t imageLength;
};

// Result of calling the external AI classification server.
struct AIClassificationResult {
    bool success; // false = HTTP/JSON failure, not "no object found"
    char object[24];
    float confidence;
};
