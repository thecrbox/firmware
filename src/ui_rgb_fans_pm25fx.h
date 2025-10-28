#pragma once
#include "esphome.h"
#include <array>
#include "esphome/components/light/addressable_light.h"
#include "ui_rgb_pm25fx_common.h"

using namespace esphome;

namespace pm25fx {

inline auto make_fan_layout(int segs, int leds_per_seg) {
  static const std::array<std::array<int, 2>, 5> segment_desc{{
      {7, 7},
      {0, 6},
      {1, 5},
      {2, 4},
      {3, 3},
  }};

  return make_layout(segs, leds_per_seg, static_cast<int>(segment_desc.size()),
                     [leds_per_seg, &segment_desc](int seg, int row, auto &&cb) {
                       int base = seg * leds_per_seg;
                       const auto &positions = segment_desc[row];
                       for (int pos : positions) {
                         cb(base + pos);
                       }
                     });
}

} // namespace pm25fx

namespace thebox_light_effects {

void pm25_boot_sweep(esphome::light::AddressableLight &it, float fanlevel) {
    // === Inputs / globals ===
    const int   kSegs        = 5;
    const int   kLedsPerSeg  = 8;
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

    const auto layout = pm25fx::make_fan_layout(kSegs, kLedsPerSeg);
    pm25fx::render(it, layout,
                   v, fanlevel,
                    kMaxCol, kMaxBri,
                    /*cut_value*/ v,
                    /*green_threshold*/ kGreenThresh);
    it.schedule_show();
}

void pm25_state(esphome::light::AddressableLight &it, float value, float fanlevel) {
    // === Inputs / globals ===
    const int   kSegs        = 5;
    const int   kLedsPerSeg  = 8;
    const float kMaxCol      = 50.0f;
    const float kMaxBri      = 50.0f;
    const float kGreenThresh = 5.0f;

    const auto layout = pm25fx::make_fan_layout(kSegs, kLedsPerSeg);
    pm25fx::render(it, layout,
                   value, fanlevel,
                    kMaxCol, kMaxBri,
                    /*cut_value*/ value,
                    /*green_threshold*/ kGreenThresh);
    it.schedule_show();
}

} // namespace thebox_light_effects
