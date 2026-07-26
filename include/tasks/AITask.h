#pragma once

// ============================================================================
// AITask.h — Consumes CameraCaptureResult from Resources::imageQueue,
// uploads it to the AI server, and forwards a TelegramMessage only when the
// response is a confident dog detection. Owns the image buffer's lifetime:
// frees it unless ownership is handed to TelegramTask for a photo message.
// ============================================================================

void AITask(void* pvParameters);
