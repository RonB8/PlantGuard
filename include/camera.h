#pragma once

// ============================================================================
// camera.h — CameraManager: thin wrapper around the esp32-camera driver.
//
// Pin mapping targets an OV2640 module wired directly to an ESP32 DevKit V1
// (see README "Design Decisions" for why this project does not use a
// separate AI-Thinker ESP32-CAM board). Pins were chosen to:
//   - keep GPIO34/35/36/39 (input-only) as camera data lines,
//   - leave GPIO33 (ADC1_CH5) free for the soil moisture sensor,
//   - leave GPIO13/14 free for a dedicated VL53L0X I2C bus,
//   - avoid GPIO21/22 (used for camera SCCB here) so the VL53L0X bus never
//     shares an I2C controller with the camera.
// Cross-reference docs/wiring_diagram.md before physically wiring this up.
// ============================================================================

#include <cstddef>
#include <cstdint>

class CameraManager {
public:
    // Configures and powers up the OV2640. Returns false (and logs the
    // esp_err_t) if the sensor does not respond — callers should treat this
    // as a fatal-for-this-feature condition and keep retrying periodically
    // rather than crash the whole system.
    static bool init();

    // Captures one JPEG frame and copies it into a freshly heap-allocated
    // buffer (caller owns it, must free() it). Returns false on timeout or
    // allocation failure; *outBuffer/*outLength are left untouched on
    // failure. Copying out of the driver's internal frame buffer
    // immediately and returning it to the driver keeps CameraTask's hold
    // time on the (single, PSRAM-less) frame buffer as short as possible.
    static bool capture(uint8_t** outBuffer, size_t* outLength);

    static bool isInitialized();
};
