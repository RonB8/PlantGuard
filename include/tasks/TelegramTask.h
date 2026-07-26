#pragma once

// ============================================================================
// TelegramTask.h — Sole consumer of Resources::telegramQueue. Sends either a
// plain text alert or a photo-with-caption, then frees any attached image
// buffer it was handed ownership of, regardless of send outcome.
// ============================================================================

void TelegramTask(void* pvParameters);
