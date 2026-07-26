#pragma once

// ============================================================================
// SensorTask.h — Owns both physical sensors (soil moisture ADC + VL53L0X).
//
// Runs a single fast loop (tick = Config::DISTANCE_POLL_INTERVAL_MS) so the
// distance sensor gets responsive, near-real-time sampling for the "closer
// than 50cm for >=1s" rule, while soil moisture is only actually read once
// every Config::SOIL_READ_INTERVAL_MS using an elapsed-time check against
// millis() — not a second vTaskDelay — so one task cleanly serves both
// features without blocking either.
//
// Outputs:
//  - TelegramMessage -> Resources::telegramQueue   (low moisture alert)
//  - captureSemaphore "give"                        (sustained close object)
// ============================================================================

void SensorTask(void* pvParameters);
