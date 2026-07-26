#pragma once

// ============================================================================
// moisture_utils.h — Pure ADC-to-percentage conversion logic.
//
// Deliberately hardware-free (no Arduino.h) so it can be exercised by the
// native unit-test environment (`pio test -e native`) without a board or
// mocked peripherals attached. SensorTask.cpp is the only caller in the
// firmware; it hands over the raw analogRead() value and calibration
// constants from config.h.
// ============================================================================

namespace MoistureUtils {

// Converts a raw ADC reading into a 0-100 moisture percentage.
//
// The capacitive sensor reads HIGH in dry air and LOW submerged in water,
// so the mapping is inverted relative to a naive linear scale. Result is
// clamped to [0, 100] to absorb sensor noise slightly outside the
// calibrated endpoints.
float rawToPercent(int rawAdc, int rawDry, int rawWet);

// Basic plausibility check to flag a disconnected/shorted sensor: a real
// capacitive probe never reports outside this range on a healthy ADC.
bool isRawValueValid(int rawAdc, int minValid, int maxValid);

} // namespace MoistureUtils
