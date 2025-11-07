#pragma once

#include "esphome.h"
#include <array>
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

inline auto make_fan_layout(int segs, int leds_per_seg) {
  static const std::array<std::array<int, 2>, 5> segment_desc{{
      {7, 7},
      {0, 6},
      {1, 5},
      {2, 4},
      {3, 3},
  }};

  return make_layout(segs, leds_per_seg, static_cast<int>(segment_desc.size()),
                     [leds_per_seg](int seg, int row, auto &&cb) {
                       int base = seg * leds_per_seg;
                       const auto &positions = segment_desc[row];
                       for (int pos : positions) {
                         cb(base + pos);
                       }
                     });
}

}  // namespace pm25fx

namespace thebox_light_effects {

inline auto make_strip_layout() {
  static constexpr int kSegs = 1;
  static constexpr int kLedsPerSeg = 30;
  static constexpr int kHeightPerSeg = kLedsPerSeg;
  return pm25fx::make_strip_layout(kSegs, kLedsPerSeg, kHeightPerSeg);
}

inline auto make_fan_layout(int segs, int leds_per_seg) {
  return pm25fx::make_fan_layout(segs, leds_per_seg);
}

inline void pm25_boot_sweep_strip(esphome::light::AddressableLight &it, float fanlevel) {
  const float kMaxCol      = 50.0f;
  const float kMaxBri      = 50.0f;
  const float kGreenThresh = 5.0f;

  static int step = -1;
  step++;

  const int period = static_cast<int>(2 * kMaxCol);
  int t = step % period;
  float v = (t <= kMaxCol) ? static_cast<float>(t) : (2 * kMaxCol - static_cast<float>(t));

  const auto layout = make_strip_layout();
  pm25fx::render(it, layout,
                 v, fanlevel,
                 kMaxCol, kMaxBri,
                 /*cut_value*/ v,
                 /*green_threshold*/ kGreenThresh);
  it.schedule_show();
}

inline void pm25_state_strip(esphome::light::AddressableLight &it, float value, float fanlevel) {
  const float kMaxCol      = 50.0f;
  const float kMaxBri      = 50.0f;
  const float kGreenThresh = 5.0f;

  const auto layout = make_strip_layout();
  pm25fx::render(it, layout,
                 value, fanlevel,
                 kMaxCol, kMaxBri,
                 /*cut_value*/ value,
                 /*green_threshold*/ kGreenThresh);
  it.schedule_show();
}

inline void pm25_boot_sweep_fans(esphome::light::AddressableLight &it, float fanlevel) {
  static constexpr int kSegs       = 5;
  static constexpr int kLedsPerSeg = 8;
  const float kMaxCol      = 50.0f;
  const float kMaxBri      = 50.0f;
  const float kGreenThresh = 5.0f;

  static int step = -1;
  step++;

  const int period = static_cast<int>(2 * kMaxCol);
  int t = step % period;
  float v = (t <= kMaxCol) ? static_cast<float>(t) : (2 * kMaxCol - static_cast<float>(t));

  const auto layout = make_fan_layout(kSegs, kLedsPerSeg);
  pm25fx::render(it, layout,
                 v, fanlevel,
                 kMaxCol, kMaxBri,
                 /*cut_value*/ v,
                 /*green_threshold*/ kGreenThresh);
  it.schedule_show();
}

inline void pm25_state_fans(esphome::light::AddressableLight &it, float value, float fanlevel) {
  static constexpr int kSegs       = 5;
  static constexpr int kLedsPerSeg = 8;
  const float kMaxCol      = 50.0f;
  const float kMaxBri      = 50.0f;
  const float kGreenThresh = 5.0f;

  const auto layout = make_fan_layout(kSegs, kLedsPerSeg);
  pm25fx::render(it, layout,
                 value, fanlevel,
                 kMaxCol, kMaxBri,
                 /*cut_value*/ value,
                 /*green_threshold*/ kGreenThresh);
  it.schedule_show();
}

inline void pm25_boot_sweep_single_fan(esphome::light::AddressableLight &it, float fanlevel) {
  static constexpr int kSegs       = 1;
  static constexpr int kLedsPerSeg = 8;
  const float kMaxCol      = 50.0f;
  const float kMaxBri      = 50.0f;
  const float kGreenThresh = 5.0f;

  static int step = -1;
  step++;

  const int period = static_cast<int>(2 * kMaxCol);
  int t = step % period;
  float v = (t <= kMaxCol) ? static_cast<float>(t) : (2 * kMaxCol - static_cast<float>(t));

  const auto layout = make_fan_layout(kSegs, kLedsPerSeg);
  pm25fx::render(it, layout,
                 v, fanlevel,
                 kMaxCol, kMaxBri,
                 /*cut_value*/ v,
                 /*green_threshold*/ kGreenThresh);
  it.schedule_show();
}

inline void pm25_state_single_fan(esphome::light::AddressableLight &it, float value, float fanlevel) {
  static constexpr int kSegs       = 1;
  static constexpr int kLedsPerSeg = 8;
  const float kMaxCol      = 50.0f;
  const float kMaxBri      = 50.0f;
  const float kGreenThresh = 5.0f;

  const auto layout = make_fan_layout(kSegs, kLedsPerSeg);
  pm25fx::render(it, layout,
                 value, fanlevel,
                 kMaxCol, kMaxBri,
                 /*cut_value*/ value,
                 /*green_threshold*/ kGreenThresh);
  it.schedule_show();
}

}  // namespace thebox_light_effects

