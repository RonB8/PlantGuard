#pragma once

// ============================================================================
// telegram.h — Telegram Bot API client (sendMessage / sendPhoto).
// ============================================================================

#include <cstddef>
#include <cstdint>

class TelegramClient {
public:
    // Sends a plain text message to Config::TELEGRAM_CHAT_ID. Retries up to
    // Config::TELEGRAM_MAX_RETRIES times with a fixed backoff on failure.
    static bool sendMessage(const char* text);

    // Sends `text` as a photo caption with the JPEG in [buffer, buffer+length)
    // attached, via multipart/form-data. Same retry policy as sendMessage.
    static bool sendPhoto(const char* caption, const uint8_t* buffer, size_t length);
};
