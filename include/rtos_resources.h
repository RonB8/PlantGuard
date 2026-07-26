#pragma once

// ============================================================================
// rtos_resources.h — Declares every inter-task communication primitive.
//
// This is the ONE place that owns queue/semaphore/event-group handles, so
// there is exactly one definition (in rtos_resources.cpp) and every task
// file gets them via `extern` instead of passing pointers around or reaching
// into global variables scattered across the codebase. This keeps the
// "avoid global shared variables" requirement honest: the only globals are
// RTOS handles, never raw sensor state.
//
// Queue map
// ---------
//   logQueue       LogMessage           any task        -> LoggerTask
//   imageQueue     CameraCaptureResult  CameraTask       -> AITask
//   telegramQueue  TelegramMessage      SensorTask/AITask-> TelegramTask
//
// Semaphore
// ---------
//   captureSemaphore   binary semaphore, SensorTask "gives" it the instant
//                      the distance state machine confirms a sustained
//                      close-range object; CameraTask blocks on "take".
//                      A semaphore (not a queue) is used because no payload
//                      is needed — it is a pure event signal.
//
// Event group
// -----------
//   systemEvents  WIFI_CONNECTED_BIT tracks link state so AITask/TelegramTask
//                 can skip network I/O instead of blocking on a dead socket.
// ============================================================================

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "types.h"

namespace Resources {

extern QueueHandle_t logQueue;
extern QueueHandle_t imageQueue;
extern QueueHandle_t telegramQueue;

extern SemaphoreHandle_t captureSemaphore;

extern EventGroupHandle_t systemEvents;
constexpr EventBits_t WIFI_CONNECTED_BIT = BIT0;

// Depths chosen generously relative to producer rate — see README for the
// reasoning behind each number (image/telegram queues only ever need to
// hold a couple of in-flight items given one camera and one uplink).
constexpr UBaseType_t LOG_QUEUE_LEN      = 16;
constexpr UBaseType_t IMAGE_QUEUE_LEN    = 2;
constexpr UBaseType_t TELEGRAM_QUEUE_LEN = 4;

// Creates every queue/semaphore/event group. Must be called once from
// main(), before any task that touches these handles is created.
void createAll();

} // namespace Resources
