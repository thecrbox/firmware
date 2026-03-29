#pragma once

namespace Config {
    static constexpr int kWarmupDurationS = WARMUP_DURATION_S;
    static constexpr float kUpdateIntervalBoot_s = ((float)UPDATE_INTERVAL_BOOT_MS/(float)1000);
    static constexpr float kUpdateIntervalNormal_s = ((float)UPDATE_INTERVAL_NORMAL_MS/(float)1000);
}
