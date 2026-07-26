#pragma once

// ============================================================================
// wifi.h — WiFiManager: connection lifecycle + automatic reconnect.
//
// Rather than a dedicated FreeRTOS task, connectivity is handled through the
// asynchronous WiFi event callback the esp32 Arduino core already runs on
// its own internal task. WiFiManager just wires that callback to
// Resources::systemEvents (WIFI_CONNECTED_BIT) so any task can cheaply
// check/wait on link state, and schedules reconnect attempts with a capped
// exponential backoff — satisfying "automatic reconnect" without adding a
// polling task or ever calling delay().
// ============================================================================

#include <WiFi.h>
#include <cstdint>

class WiFiManager {
public:
    // Starts the initial connection attempt (blocks the calling task only
    // for the connect handshake, bounded by Config::WIFI_CONNECT_TIMEOUT_MS,
    // using vTaskDelay-based polling — never delay()).
    static void begin();

    // Non-blocking: true if the last known link state is "connected".
    static bool isConnected();

    // Call periodically (e.g. once per SensorTask tick) from a task that is
    // already running its own loop. Cheap no-op unless a reconnect is due.
    static void poll();

private:
    static void onWiFiEvent(WiFiEvent_t event);
};
