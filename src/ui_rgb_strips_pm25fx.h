#pragma once
#include "esphome.h"
#include "esphome/components/light/addressable_light.h"
#include "ui_rgb_pm25fx_common.h"

using namespace esphome;

namespace pm25fx {

inline auto make_strip_layout(int segs, int leds_per_seg, int height_per_seg) {
  return make_layout(segs, leds_per_seg, height_per_seg,
                     [leds_per_seg](int seg, int row, auto &&cb) {
                       cb(seg * leds_per_seg + row);
                     });
}

} // namespace pm25fx

namespace thebox_light_effects {

void pm25_boot_sweep(esphome::light::AddressableLight &it, float fanlevel) {
    // === Inputs / globals ===
    const int   kSegs        = 1;
    const int   kLedsPerSeg  = 30;
    const float kMaxCol      = 50.0f;
    const float kMaxBri      = 50.0f;
    const float kGreenThresh = 5.0f;
    // =========================

    static int step = -1;   // start at -1 to make first frame 0 (EXACT)
    step++;

    // triangle wave 0..40..0 across a period of 80 (EXACT)
    const int period = (int)(2 * kMaxCol);   // 80
    int t = step % period;                   // 0..79
    float v = (t <= kMaxCol) ? (float)t : (2*kMaxCol - (float)t);

    const auto layout = pm25fx::make_strip_layout(kSegs, kLedsPerSeg, /*height_per_seg*/ kLedsPerSeg);
    pm25fx::render(it, layout,
                   v, fanlevel,
                    kMaxCol, kMaxBri,
                    /*cut_value*/ v,
                    /*green_threshold*/ kGreenThresh);
    it.schedule_show();
}

void pm25_state(esphome::light::AddressableLight &it, float value, float fanlevel) {
    // === Inputs / globals ===
    const int   kSegs        = 1;
    const int   kLedsPerSeg  = 30;
    const float kMaxCol      = 50.0f;
    const float kMaxBri      = 50.0f;
    const float kGreenThresh = 5.0f;

    const auto layout = pm25fx::make_strip_layout(kSegs, kLedsPerSeg, /*height_per_seg*/ kLedsPerSeg);
    pm25fx::render(it, layout,
                   value, fanlevel,
                    kMaxCol, kMaxBri,
                    /*cut_value*/ value,
                    /*green_threshold*/ kGreenThresh);
    it.schedule_show();
}

} // namespace thebox_light_effects
