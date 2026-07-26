#include "tasks/SensorTask.h"

#include <Arduino.h>
#include <VL53L0X.h>
#include <Wire.h>
#include "esp_task_wdt.h"

#include "config.h"
#include "detection_logic.h"
#include "logging.h"
#include "moisture_utils.h"
#include "rtos_resources.h"

namespace {

VL53L0X g_rangeSensor;
bool g_distanceSensorOk = false;
uint32_t g_nextDistanceInitRetryMs = 0;

// Attempts (re)initialization of the VL53L0X. Safe to call repeatedly.
bool initDistanceSensor() {
    Wire.begin(Config::I2C_SDA_PIN, Config::I2C_SCL_PIN);
    g_rangeSensor.setTimeout(500);

    if (!g_rangeSensor.init()) {
        LOG_ERROR("VL53L0X init failed (check wiring on SDA=%d SCL=%d)",
                   Config::I2C_SDA_PIN, Config::I2C_SCL_PIN);
        return false;
    }

    g_rangeSensor.startContinuous();
    LOG_INFO("VL53L0X distance sensor ready");
    return true;
}

void handleMoistureReading(bool& wateringAlertActive) {
    int raw = analogRead(Config::SOIL_MOISTURE_ADC_PIN);

    if (!MoistureUtils::isRawValueValid(raw, Config::SOIL_ADC_RAW_MIN_VALID,
                                         Config::SOIL_ADC_RAW_MAX_VALID)) {
        LOG_ERROR("Soil moisture sensor: implausible raw ADC value %d (check wiring)", raw);
        return;
    }

    float percent = MoistureUtils::rawToPercent(raw, Config::SOIL_ADC_RAW_DRY, Config::SOIL_ADC_RAW_WET);
    LOG_INFO("Moisture %.1f%% (raw=%d)", percent, raw);

    bool needsWater = percent <= Config::SOIL_MOISTURE_THRESHOLD_PERCENT;

    if (needsWater && !wateringAlertActive) {
        // Edge-triggered: only fires the instant we cross below threshold.
        wateringAlertActive = true;
        TelegramMessage msg = {};
        snprintf(msg.text, sizeof(msg.text),
                 "\xF0\x9F\xAA\xB4 Plant needs watering.\nCurrent soil moisture: %.0f%%", percent);
        msg.imageBuffer = nullptr;
        msg.imageLength = 0;
        if (xQueueSend(Resources::telegramQueue, &msg, 0) != pdTRUE) {
            LOG_WARN("Telegram queue full, dropping watering alert");
        }
    } else if (!needsWater && wateringAlertActive) {
        wateringAlertActive = false;
        LOG_INFO("Soil moisture recovered above threshold, watering alert re-armed");
    }
}

void handleDistanceReading(DetectionLogic::DistanceStateMachine& stateMachine) {
    uint32_t now = millis();

    if (!g_distanceSensorOk) {
        if (static_cast<int32_t>(now - g_nextDistanceInitRetryMs) >= 0) {
            g_distanceSensorOk = initDistanceSensor();
            g_nextDistanceInitRetryMs = now + Config::DISTANCE_SENSOR_INIT_RETRY_MS;
        }
        return;
    }

    uint16_t distanceMm = g_rangeSensor.readRangeContinuousMillimeters();
    if (g_rangeSensor.timeoutOccurred()) {
        LOG_WARN("VL53L0X read timeout, will retry next tick");
        return; // skip this sample rather than feeding a bad reading in
    }

    bool isClose = distanceMm < Config::DISTANCE_THRESHOLD_MM;
    LOG_DEBUG("Distance %u mm", distanceMm);

    auto action = stateMachine.update(isClose, now);
    if (action == DetectionLogic::Action::TRIGGER) {
        LOG_INFO("Object sustained closer than %u mm for %lu ms — triggering camera",
                 Config::DISTANCE_THRESHOLD_MM,
                 static_cast<unsigned long>(Config::DISTANCE_SUSTAIN_MS));
        xSemaphoreGive(Resources::captureSemaphore);
    }
}

} // namespace

void SensorTask(void* pvParameters) {
    (void)pvParameters;
    esp_task_wdt_add(nullptr);

    analogReadResolution(12); // 0-4095, matches Config::SOIL_ADC_RAW_* calibration

    g_distanceSensorOk = initDistanceSensor();
    g_nextDistanceInitRetryMs = millis() + Config::DISTANCE_SENSOR_INIT_RETRY_MS;

    DetectionLogic::DistanceStateMachine distanceSM(Config::DISTANCE_SUSTAIN_MS,
                                                      Config::DOG_DETECTION_COOLDOWN_MS);
    bool wateringAlertActive = false;
    uint32_t nextMoistureReadMs = millis(); // read immediately on boot

    TickType_t lastWake = xTaskGetTickCount();
    for (;;) {
        uint32_t now = millis();

        handleDistanceReading(distanceSM);

        if (static_cast<int32_t>(now - nextMoistureReadMs) >= 0) {
            handleMoistureReading(wateringAlertActive);
            nextMoistureReadMs = now + Config::SOIL_READ_INTERVAL_MS;
        }

        esp_task_wdt_reset();
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(Config::DISTANCE_POLL_INTERVAL_MS));
    }
}
