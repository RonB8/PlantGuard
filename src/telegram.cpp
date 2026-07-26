#include "telegram.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "config.h"
#include "logging.h"

namespace {

// Percent-encodes a string for safe inclusion in a URL query component.
// Telegram message text can contain emoji/spaces/newlines (the spec's
// notification text does), so this can't be skipped.
String urlEncode(const char* src) {
    String out;
    out.reserve(strlen(src) * 3);
    const char* hex = "0123456789ABCDEF";
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(src); *p; ++p) {
        if (isalnum(*p) || *p == '-' || *p == '_' || *p == '.' || *p == '~') {
            out += static_cast<char>(*p);
        } else {
            out += '%';
            out += hex[(*p) >> 4];
            out += hex[(*p) & 0x0F];
        }
    }
    return out;
}

bool trySendMessageOnce(const char* text) {
    WiFiClientSecure client;
    client.setInsecure(); // see README "Security Notes": no cert pinning in this build

    HTTPClient http;
    http.setTimeout(Config::TELEGRAM_HTTP_TIMEOUT_MS);

    String url = String("https://api.telegram.org/bot") + Config::TELEGRAM_BOT_TOKEN + "/sendMessage";
    if (!http.begin(client, url)) {
        LOG_ERROR("Telegram sendMessage: http.begin() failed");
        return false;
    }
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String body = String("chat_id=") + Config::TELEGRAM_CHAT_ID + "&text=" + urlEncode(text);
    int status = http.POST(body);
    http.end();

    if (status != HTTP_CODE_OK) {
        LOG_WARN("Telegram sendMessage: HTTP %d", status);
        return false;
    }
    return true;
}

bool trySendPhotoOnce(const char* caption, const uint8_t* buffer, size_t length) {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setTimeout(Config::TELEGRAM_HTTP_TIMEOUT_MS);

    String url = String("https://api.telegram.org/bot") + Config::TELEGRAM_BOT_TOKEN + "/sendPhoto";
    if (!http.begin(client, url)) {
        LOG_ERROR("Telegram sendPhoto: http.begin() failed");
        return false;
    }

    const char* boundary = "SmartPlantGuardianBoundary";

    String head;
    head += "--"; head += boundary; head += "\r\n";
    head += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
    head += Config::TELEGRAM_CHAT_ID;
    head += "\r\n--"; head += boundary; head += "\r\n";
    head += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n";
    head += caption;
    head += "\r\n--"; head += boundary; head += "\r\n";
    head += "Content-Disposition: form-data; name=\"photo\"; filename=\"capture.jpg\"\r\n";
    head += "Content-Type: image/jpeg\r\n\r\n";

    String tail = String("\r\n--") + boundary + "--\r\n";

    size_t totalLen = head.length() + length + tail.length();
    uint8_t* multipartBody = static_cast<uint8_t*>(malloc(totalLen));
    if (multipartBody == nullptr) {
        LOG_ERROR("Telegram sendPhoto: out of memory assembling %u-byte multipart body",
                   static_cast<unsigned>(totalLen));
        http.end();
        return false;
    }

    size_t offset = 0;
    memcpy(multipartBody + offset, head.c_str(), head.length()); offset += head.length();
    memcpy(multipartBody + offset, buffer, length); offset += length;
    memcpy(multipartBody + offset, tail.c_str(), tail.length());

    http.addHeader("Content-Type", String("multipart/form-data; boundary=") + boundary);
    int status = http.POST(multipartBody, totalLen);

    free(multipartBody);
    http.end();

    if (status != HTTP_CODE_OK) {
        LOG_WARN("Telegram sendPhoto: HTTP %d", status);
        return false;
    }
    return true;
}

} // namespace

bool TelegramClient::sendMessage(const char* text) {
    if (WiFi.status() != WL_CONNECTED) {
        LOG_WARN("Telegram: skipping sendMessage, WiFi not connected");
        return false;
    }

    for (uint8_t attempt = 1; attempt <= Config::TELEGRAM_MAX_RETRIES + 1; ++attempt) {
        if (trySendMessageOnce(text)) return true;
        if (attempt <= Config::TELEGRAM_MAX_RETRIES) {
            vTaskDelay(pdMS_TO_TICKS(Config::TELEGRAM_RETRY_BACKOFF_MS));
        }
    }
    LOG_ERROR("Telegram sendMessage failed after %u attempts", Config::TELEGRAM_MAX_RETRIES + 1);
    return false;
}

bool TelegramClient::sendPhoto(const char* caption, const uint8_t* buffer, size_t length) {
    if (WiFi.status() != WL_CONNECTED) {
        LOG_WARN("Telegram: skipping sendPhoto, WiFi not connected");
        return false;
    }

    for (uint8_t attempt = 1; attempt <= Config::TELEGRAM_MAX_RETRIES + 1; ++attempt) {
        if (trySendPhotoOnce(caption, buffer, length)) return true;
        if (attempt <= Config::TELEGRAM_MAX_RETRIES) {
            vTaskDelay(pdMS_TO_TICKS(Config::TELEGRAM_RETRY_BACKOFF_MS));
        }
    }
    LOG_ERROR("Telegram sendPhoto failed after %u attempts", Config::TELEGRAM_MAX_RETRIES + 1);
    return false;
}
