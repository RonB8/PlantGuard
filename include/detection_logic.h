#pragma once

// ============================================================================
// detection_logic.h — Pure decision logic for the dog-detection pipeline.
//
// Two hardware-free pieces live here, both unit-tested natively:
//
//  1. DistanceStateMachine — turns a stream of "is something within
//     threshold?" samples into a single TRIGGER edge once the object has
//     been continuously present for DISTANCE_SUSTAIN_MS, then enforces a
//     cooldown so a lingering animal doesn't fire the camera repeatedly.
//
//  2. isDogDetected() — the AI-response acceptance rule (object == "dog"
//     AND confidence > threshold), isolated so it can be tested against
//     edge cases (case sensitivity, boundary confidence) without an HTTP
//     stack.
// ============================================================================

#include <cstdint>
#include <cstring>

namespace DetectionLogic {

enum class Action { NONE, TRIGGER };

class DistanceStateMachine {
public:
    DistanceStateMachine(uint32_t sustainMs, uint32_t cooldownMs)
        : sustainMs_(sustainMs), cooldownMs_(cooldownMs) {}

    // Feed one sample. `isClose` = latest reading is below the distance
    // threshold. `nowMs` = monotonic milliseconds (e.g. millis()).
    // Returns Action::TRIGGER exactly once per qualifying close-range
    // episode (i.e. on the sample where the sustain window is satisfied).
    Action update(bool isClose, uint32_t nowMs) {
        switch (state_) {
            case State::IDLE:
                if (isClose) {
                    state_ = State::CANDIDATE;
                    candidateStartMs_ = nowMs;
                }
                return Action::NONE;

            case State::CANDIDATE:
                if (!isClose) {
                    state_ = State::IDLE;
                    return Action::NONE;
                }
                if (elapsedSince(candidateStartMs_, nowMs) >= sustainMs_) {
                    state_ = State::COOLDOWN;
                    cooldownStartMs_ = nowMs;
                    return Action::TRIGGER;
                }
                return Action::NONE;

            case State::COOLDOWN:
                if (elapsedSince(cooldownStartMs_, nowMs) >= cooldownMs_) {
                    // Re-arm. If the object is still close, start a fresh
                    // candidate window immediately rather than requiring a
                    // full IDLE round-trip.
                    state_ = isClose ? State::CANDIDATE : State::IDLE;
                    candidateStartMs_ = nowMs;
                }
                return Action::NONE;
        }
        return Action::NONE; // unreachable, silences -Wreturn-type
    }

    bool isIdle() const { return state_ == State::IDLE; }

private:
    enum class State { IDLE, CANDIDATE, COOLDOWN };

    static uint32_t elapsedSince(uint32_t startMs, uint32_t nowMs) {
        // Unsigned subtraction is intentional: correctly handles millis()
        // rollover (wraps every ~49.7 days) without special-casing it.
        return nowMs - startMs;
    }

    State state_ = State::IDLE;
    uint32_t candidateStartMs_ = 0;
    uint32_t cooldownStartMs_ = 0;
    const uint32_t sustainMs_;
    const uint32_t cooldownMs_;
};

// Accepts only an exact (case-sensitive) match on targetObject with
// strictly-greater-than confidence, matching the spec's "confidence > 0.9".
inline bool isDogDetected(const char* object, float confidence,
                           const char* targetObject, float confidenceThreshold) {
    if (object == nullptr || targetObject == nullptr) return false;
    return std::strcmp(object, targetObject) == 0 && confidence > confidenceThreshold;
}

} // namespace DetectionLogic
