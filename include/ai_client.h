#pragma once

// ============================================================================
// ai_client.h — HTTP client for the external AI classification server.
//
// This firmware does NOT implement the model. It only implements the
// transport: POST the JPEG as the raw request body, expect a JSON response
// shaped like:
//   { "object": "dog", "confidence": 0.96 }
// See README "AI Server API" for the full contract.
// ============================================================================

#include <cstddef>
#include <cstdint>

#include "types.h"

class AIClient {
public:
    // Uploads the JPEG in [buffer, buffer+length) to Config::AI_SERVER_URL
    // and parses the JSON response. Retries up to Config::AI_MAX_RETRIES
    // times with a fixed backoff (vTaskDelay, never delay()) on transport
    // failure or non-200 responses.
    //
    // Returns true and fills `outResult` only when a well-formed JSON
    // response was received (regardless of what object it names — a
    // successfully-parsed "cat" result is `success = true`). Returns false
    // on HTTP/timeout/malformed-JSON failure after retries are exhausted;
    // `outResult` is untouched in that case.
    static bool classify(const uint8_t* buffer, size_t length, AIClassificationResult& outResult);
};
