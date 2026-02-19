#pragma once

#include "esphome.h"
#include <array>
#include <vector> // Added for std::vector
#include <numeric> // Added for std::iota
#include "esphome/components/light/addressable_light.h"
#include "ui_rgb_aqifx_common.h"

using namespace esphome;

namespace aqifx {

inline std::vector<std::vector<int>> generate_fan_segment_description(int leds_per_seg) {
  std::vector<std::vector<int>> segment_desc;

  if (leds_per_seg <= 0) return segment_desc;

  if (leds_per_seg == 1) { // Special case for single LED
    segment_desc.push_back({0, 0});
    return segment_desc;
  }

  // Topmost LED (index leds_per_seg - 1)
  segment_desc.push_back({leds_per_seg - 1, leds_per_seg - 1});

  // Middle pairs
  int num_middle_pairs_indices = (leds_per_seg - 2) / 2; // e.g. for 8 -> 3 (0,1,2)
  for (int i = 0; i < num_middle_pairs_indices; ++i) {
    segment_desc.push_back({i, leds_per_seg - 2 - i});
  }

  // Bottommost LED (only for even leds_per_seg or if there's a distinct central LED for odd)
  if (leds_per_seg >= 2) {
      int bottom_idx = num_middle_pairs_indices; // For 8 LEDs, this is 3.
      if (leds_per_seg % 2 != 0) { // If odd leds_per_seg, it's the true central LED
          bottom_idx = (leds_per_seg - 1) / 2;
      }
      segment_desc.push_back({bottom_idx, bottom_idx});
  }

  return segment_desc;
}


// Modified make_fan_layout to accept a segment description
inline auto make_fan_layout(int segs, const std::vector<std::vector<int>>& segment_description) {
  // leds_per_seg is implicit in the segment_description, we just need its total count for the mapper.
  // Find max LED index to determine leds_per_seg
  int max_led_index = -1;
  for (const auto& row : segment_description) {
      for (int led_idx : row) {
          if (led_idx > max_led_index) {
              max_led_index = led_idx;
          }
      }
  }
  int leds_per_seg = max_led_index + 1; // Assuming 0-indexed and contiguous from 0.

  // The offsets of each fan segment in the partition.
  // These must match the number of LEDs defined for rgb_fan1, rgb_fan2, etc. in the yaml
  static const std::array<int, 5> fan_offsets = {0, 30, 30 + 30, 30 + 30 + 24, 30 + 30 + 24 + 24};

  return make_layout(segs, leds_per_seg, static_cast<int>(segment_description.size()),
                     [&segment_description](int seg, int row, auto &&cb) {
                       if (seg >= fan_offsets.size()) return;
                       int base = fan_offsets[seg];
                       const auto &positions = segment_description[row];
                       for (int pos : positions) {
                         cb(base + pos);
                       }
                     });
}

}  // namespace aqifx

namespace thebox_light_effects {

inline auto make_strip_clone_layout() {
  static constexpr int kSegs = 1; // Treat as one logical segment
  static constexpr int kLedsPerSeg = 30;
  static constexpr int kHeightPerSeg = kLedsPerSeg;
  return aqifx::make_layout(kSegs, kLedsPerSeg, kHeightPerSeg,
                            [](int seg, int row, auto &&cb) {
                              cb(row); // rgb_fan1
                              cb(row + 30); // rgb_fan2
                            });
}

inline void aqi_boot_sweep_strip_clone(esphome::light::AddressableLight &it, float fanlevel, float max_aqi_value, bool reset_animation) {
  const float kGreenThresh = 40.0f;

  static int step = -1;
  static float delta_f_cached_strip_clone = -1.0f;
  static float prev_max_aqi_strip_clone = -1.0f;
  
  if (reset_animation || prev_max_aqi_strip_clone != max_aqi_value) {
    step = -1;
    delta_f_cached_strip_clone = -1.0f;
    prev_max_aqi_strip_clone = max_aqi_value;
  }

  if (delta_f_cached_strip_clone < 0) {
      delta_f_cached_strip_clone = (max_aqi_value) * 0.05f / (aqifx::kWarmupDurationS / 2);
  }
  
  step = round((float)step + delta_f_cached_strip_clone);

  const int period = static_cast<int>(2 * max_aqi_value);
  int t = step % period;
  float v = (t <= max_aqi_value) ? static_cast<float>(t) : (2 * max_aqi_value - static_cast<float>(t));

  const auto layout = make_strip_clone_layout();
  aqifx::render(it, layout,
                 v, fanlevel,
                 max_aqi_value, max_aqi_value,
                 /*cut_value*/ v,
                 /*green_threshold*/ kGreenThresh);
  it.schedule_show();
}

inline void aqi_state_strip_clone(esphome::light::AddressableLight &it, float value, float fanlevel, float max_aqi_value) {
  const float kGreenThresh = 40.0f;

  const auto layout = make_strip_clone_layout();
  aqifx::render(it, layout,
                 value, fanlevel,
                 max_aqi_value, max_aqi_value,
                 /*cut_value*/ value,
                 /*green_threshold*/ kGreenThresh);
  it.schedule_show();
}

inline auto make_dual_strip_layout() {
  static constexpr int kSegs = 1;
  static constexpr int kLedsPerSeg = 60;
  static constexpr int kHeightPerSeg = kLedsPerSeg;
  return aqifx::make_layout(kSegs, kLedsPerSeg, kHeightPerSeg,
                            [](int seg, int row, auto &&cb) {
                              cb(row);
                            });
}

inline void aqi_boot_sweep_dual_strip(esphome::light::AddressableLight &it, float fanlevel, float max_aqi_value, bool reset_animation) {
  const float kGreenThresh = 40.0f;

  static int step = -1;
  static float delta_f_cached_dual_strip = -1.0f;
  static float prev_max_aqi_dual_strip = -1.0f;
  
  if (reset_animation || prev_max_aqi_dual_strip != max_aqi_value) {
    step = -1;
    delta_f_cached_dual_strip = -1.0f;
    prev_max_aqi_dual_strip = max_aqi_value;
  }

  if (delta_f_cached_dual_strip < 0) {
      delta_f_cached_dual_strip = (max_aqi_value) * 0.05f / (aqifx::kWarmupDurationS / 2);
  }
  
  step = round((float)step + delta_f_cached_dual_strip);

  const int period = static_cast<int>(2 * max_aqi_value);
  int t = step % period;
  float v = (t <= max_aqi_value) ? static_cast<float>(t) : (2 * max_aqi_value - static_cast<float>(t));

  const auto layout = make_dual_strip_layout();
  aqifx::render(it, layout,
                 v, fanlevel,
                 max_aqi_value, max_aqi_value,
                 /*cut_value*/ v,
                 /*green_threshold*/ kGreenThresh);
  it.schedule_show();
}

inline void aqi_state_dual_strip(esphome::light::AddressableLight &it, float value, float fanlevel, float max_aqi_value) {
  const float kGreenThresh = 40.0f;

  const auto layout = make_dual_strip_layout();
  aqifx::render(it, layout,
                 value, fanlevel,
                 max_aqi_value, max_aqi_value,
                 /*cut_value*/ value,
                 /*green_threshold*/ kGreenThresh);
  it.schedule_show();
}

// New versions of fan animation functions with parameters
inline void aqi_boot_sweep_fans(esphome::light::AddressableLight &it, int num_segs, int leds_per_seg, float max_aqi_value, float fanlevel, bool reset_animation) {
  const float kGreenThresh = 40.0f; // Can also be parameterized if needed

  static int step = -1;
  static float delta_f_cached_fans = -1.0f; // Separate cache for fans
  static float prev_max_aqi_fans = -1.0f;
  
  if (reset_animation || prev_max_aqi_fans != max_aqi_value) {
    step = -1; // Reset step to re-initialize delta_f_cached_fans
    delta_f_cached_fans = -1.0f;
    prev_max_aqi_fans = max_aqi_value;
  }

  if (delta_f_cached_fans < 0) { // Initialize or recalculate if max_aqi_value changes
      delta_f_cached_fans = (max_aqi_value) * 0.05f / (aqifx::kWarmupDurationS / 2);
  }
  
  step = round((float)step + delta_f_cached_fans);

  const int period = static_cast<int>(2 * max_aqi_value);
  int t = step % period;
  float v = (t <= max_aqi_value) ? static_cast<float>(t) : (2 * max_aqi_value - static_cast<float>(t));

  // Generate segment description dynamically
  const auto segment_description = aqifx::generate_fan_segment_description(leds_per_seg);
  const auto layout = aqifx::make_fan_layout(num_segs, segment_description);

  aqifx::render(it, layout,
                 v, fanlevel,
                 max_aqi_value, max_aqi_value,
                 /*cut_value*/ v,
                 /*green_threshold*/ kGreenThresh);
  it.schedule_show();
}

inline void aqi_state_fans(esphome::light::AddressableLight &it, int num_segs, int leds_per_seg, float max_aqi_value, float value, float fanlevel) {
  const float kGreenThresh = 40.0f; // Can also be parameterized if needed

  // Generate segment description dynamically
  const auto segment_description = aqifx::generate_fan_segment_description(leds_per_seg);
  const auto layout = aqifx::make_fan_layout(num_segs, segment_description);

  aqifx::render(it, layout,
                 value, fanlevel,
                 max_aqi_value, max_aqi_value,
                 /*cut_value*/ value,
                 /*green_threshold*/ kGreenThresh);
  it.schedule_show();
}

}  // namespace thebox_light_effects