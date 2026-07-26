#include "ai_client.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "config.h"
#include "logging.h"

namespace {

// One upload attempt. Returns true only on HTTP 200 + valid JSON body.
bool tryClassifyOnce(const uint8_t* buffer, size_t length, AIClassificationResult& outResult) {
    HTTPClient http;
    http.setTimeout(Config::AI_HTTP_TIMEOUT_MS);

    if (!http.begin(Config::AI_SERVER_URL)) {
        LOG_ERROR("AI client: http.begin() failed for %s", Config::AI_SERVER_URL);
        return false;
    }
    http.addHeader("Content-Type", "image/jpeg");

    int status = http.POST(const_cast<uint8_t*>(buffer), length);
    if (status != HTTP_CODE_OK) {
        LOG_WARN("AI server returned HTTP %d", status);
        http.end();
        return false;
    }

    String body = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError jsonErr = deserializeJson(doc, body);
    if (jsonErr) {
        LOG_ERROR("AI server response: invalid JSON (%s)", jsonErr.c_str());
        return false;
    }

    const char* object = doc["object"] | "";
    if (object[0] == '\0' || !doc["confidence"].is<float>()) {
        LOG_ERROR("AI server response: missing 'object'/'confidence' field");
        return false;
    }

    outResult.success = true;
    strncpy(outResult.object, object, sizeof(outResult.object) - 1);
    outResult.object[sizeof(outResult.object) - 1] = '\0';
    outResult.confidence = doc["confidence"].as<float>();
    return true;
}

} // namespace

bool AIClient::classify(const uint8_t* buffer, size_t length, AIClassificationResult& outResult) {
    if (WiFi.status() != WL_CONNECTED) {
        LOG_WARN("AI client: skipping upload, WiFi not connected");
        return false;
    }

    for (uint8_t attempt = 1; attempt <= Config::AI_MAX_RETRIES + 1; ++attempt) {
        if (tryClassifyOnce(buffer, length, outResult)) {
            return true;
        }
        if (attempt <= Config::AI_MAX_RETRIES) {
            LOG_WARN("AI client: retrying upload (attempt %u/%u)",
                     attempt + 1, Config::AI_MAX_RETRIES + 1);
            vTaskDelay(pdMS_TO_TICKS(Config::AI_RETRY_BACKOFF_MS));
        }
    }

    LOG_ERROR("AI client: upload failed after %u attempts", Config::AI_MAX_RETRIES + 1);
    return false;
}
