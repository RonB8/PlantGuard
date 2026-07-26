#include "wifi.h"

#include <Arduino.h>

#include "config.h"
#include "logging.h"
#include "rtos_resources.h"

namespace {

uint32_t s_nextReconnectAttemptMs = 0;
uint32_t s_currentBackoffMs = Config::WIFI_RECONNECT_RETRY_MS;

} // namespace

void WiFiManager::onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            xEventGroupSetBits(Resources::systemEvents, Resources::WIFI_CONNECTED_BIT);
            s_currentBackoffMs = Config::WIFI_RECONNECT_RETRY_MS; // reset backoff
            LOG_INFO("WiFi connected, IP=%s", WiFi.localIP().toString().c_str());
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            xEventGroupClearBits(Resources::systemEvents, Resources::WIFI_CONNECTED_BIT);
            LOG_WARN("WiFi disconnected, will retry in %lu ms",
                     static_cast<unsigned long>(s_currentBackoffMs));
            s_nextReconnectAttemptMs = millis() + s_currentBackoffMs;
            break;

        default:
            break;
    }
}

void WiFiManager::begin() {
    WiFi.onEvent(onWiFiEvent);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false); // WiFiManager::poll() drives reconnects explicitly
    WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);

    LOG_INFO("Connecting to WiFi SSID '%s'...", Config::WIFI_SSID);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           (millis() - start) < Config::WIFI_CONNECT_TIMEOUT_MS) {
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    if (WiFi.status() != WL_CONNECTED) {
        LOG_WARN("Initial WiFi connect timed out, background reconnect will keep trying");
        s_nextReconnectAttemptMs = millis() + s_currentBackoffMs;
    }
}

bool WiFiManager::isConnected() {
    return (xEventGroupGetBits(Resources::systemEvents) & Resources::WIFI_CONNECTED_BIT) != 0;
}

void WiFiManager::poll() {
    if (isConnected()) return;

    uint32_t now = millis();
    if (static_cast<int32_t>(now - s_nextReconnectAttemptMs) < 0) return;

    LOG_INFO("Attempting WiFi reconnect...");
    WiFi.disconnect();
    WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);

    // Exponential backoff, capped, so a persistently absent AP doesn't spam
    // reconnect attempts forever.
    s_currentBackoffMs = min(s_currentBackoffMs * 2, Config::WIFI_RECONNECT_MAX_BACKOFF_MS);
    s_nextReconnectAttemptMs = now + s_currentBackoffMs;
}
