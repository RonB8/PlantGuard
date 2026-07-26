// ============================================================================
// test_logic.cpp — Native Unity tests for the hardware-free logic modules.
//
// Run with: pio test -e native
//
// These cover the two pieces of business logic worth protecting with a
// regression suite: the ADC->percent conversion (moisture_utils) and the
// dog-detection decision, including the sustained-proximity state machine
// (detection_logic). Neither module includes Arduino.h, so both compile and
// run on a desktop toolchain with no board attached.
// ============================================================================

#include <unity.h>

#include "detection_logic.h"
#include "moisture_utils.h"

void setUp() {}
void tearDown() {}

// ---------------------------------------------------------------------------
// MoistureUtils::rawToPercent
// ---------------------------------------------------------------------------

void test_moisture_dry_reading_is_zero_percent() {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, MoistureUtils::rawToPercent(3000, 3000, 1200));
}

void test_moisture_wet_reading_is_hundred_percent() {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, MoistureUtils::rawToPercent(1200, 3000, 1200));
}

void test_moisture_midpoint_is_fifty_percent() {
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 50.0f, MoistureUtils::rawToPercent(2100, 3000, 1200));
}

void test_moisture_clamps_beyond_calibration_range() {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, MoistureUtils::rawToPercent(3500, 3000, 1200));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, MoistureUtils::rawToPercent(500, 3000, 1200));
}

void test_moisture_raw_validity_bounds() {
    TEST_ASSERT_TRUE(MoistureUtils::isRawValueValid(2000, 200, 4095));
    TEST_ASSERT_FALSE(MoistureUtils::isRawValueValid(50, 200, 4095));
    TEST_ASSERT_FALSE(MoistureUtils::isRawValueValid(4096, 200, 4095));
}

// ---------------------------------------------------------------------------
// DetectionLogic::isDogDetected
// ---------------------------------------------------------------------------

void test_dog_detected_above_threshold() {
    TEST_ASSERT_TRUE(DetectionLogic::isDogDetected("dog", 0.96f, "dog", 0.9f));
}

void test_dog_not_detected_at_exact_threshold() {
    // Spec requires strictly-greater-than confidence.
    TEST_ASSERT_FALSE(DetectionLogic::isDogDetected("dog", 0.90f, "dog", 0.9f));
}

void test_dog_not_detected_for_other_objects() {
    TEST_ASSERT_FALSE(DetectionLogic::isDogDetected("cat", 0.99f, "dog", 0.9f));
    TEST_ASSERT_FALSE(DetectionLogic::isDogDetected("human", 0.99f, "dog", 0.9f));
}

// ---------------------------------------------------------------------------
// DetectionLogic::DistanceStateMachine
// ---------------------------------------------------------------------------

void test_state_machine_no_trigger_before_sustain_window() {
    DetectionLogic::DistanceStateMachine sm(1000, 30000);
    TEST_ASSERT_TRUE(sm.update(true, 0) == DetectionLogic::Action::NONE);
    TEST_ASSERT_TRUE(sm.update(true, 500) == DetectionLogic::Action::NONE);
    TEST_ASSERT_TRUE(sm.update(true, 999) == DetectionLogic::Action::NONE);
}

void test_state_machine_triggers_once_sustain_window_elapses() {
    DetectionLogic::DistanceStateMachine sm(1000, 30000);
    sm.update(true, 0);
    TEST_ASSERT_TRUE(sm.update(true, 1000) == DetectionLogic::Action::TRIGGER);
}

void test_state_machine_resets_if_object_leaves_before_sustain_window() {
    DetectionLogic::DistanceStateMachine sm(1000, 30000);
    sm.update(true, 0);
    sm.update(false, 500); // object leaves early
    TEST_ASSERT_TRUE(sm.update(true, 600) == DetectionLogic::Action::NONE); // fresh window starts
    TEST_ASSERT_TRUE(sm.update(true, 1600) == DetectionLogic::Action::TRIGGER);
}

void test_state_machine_suppresses_retrigger_during_cooldown() {
    DetectionLogic::DistanceStateMachine sm(1000, 30000);
    sm.update(true, 0);
    TEST_ASSERT_TRUE(sm.update(true, 1000) == DetectionLogic::Action::TRIGGER);
    // Object never left; still within cooldown window.
    TEST_ASSERT_TRUE(sm.update(true, 5000) == DetectionLogic::Action::NONE);
    TEST_ASSERT_TRUE(sm.update(true, 30999) == DetectionLogic::Action::NONE);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_moisture_dry_reading_is_zero_percent);
    RUN_TEST(test_moisture_wet_reading_is_hundred_percent);
    RUN_TEST(test_moisture_midpoint_is_fifty_percent);
    RUN_TEST(test_moisture_clamps_beyond_calibration_range);
    RUN_TEST(test_moisture_raw_validity_bounds);
    RUN_TEST(test_dog_detected_above_threshold);
    RUN_TEST(test_dog_not_detected_at_exact_threshold);
    RUN_TEST(test_dog_not_detected_for_other_objects);
    RUN_TEST(test_state_machine_no_trigger_before_sustain_window);
    RUN_TEST(test_state_machine_triggers_once_sustain_window_elapses);
    RUN_TEST(test_state_machine_resets_if_object_leaves_before_sustain_window);
    RUN_TEST(test_state_machine_suppresses_retrigger_during_cooldown);
    return UNITY_END();
}
