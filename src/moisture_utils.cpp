#include "moisture_utils.h"

namespace MoistureUtils {

float rawToPercent(int rawAdc, int rawDry, int rawWet) {
    // rawDry (e.g. 3000) maps to 0%, rawWet (e.g. 1200) maps to 100%.
    float span = static_cast<float>(rawDry - rawWet);
    if (span == 0.0f) {
        return 0.0f; // degenerate calibration, avoid div-by-zero
    }

    float percent = (static_cast<float>(rawDry - rawAdc) / span) * 100.0f;

    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    return percent;
}

bool isRawValueValid(int rawAdc, int minValid, int maxValid) {
    return rawAdc >= minValid && rawAdc <= maxValid;
}

} // namespace MoistureUtils
