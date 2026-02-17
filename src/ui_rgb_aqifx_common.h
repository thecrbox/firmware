#pragma once

#include "esphome.h"
#include <cmath>
#include <utility>
#include "esphome/components/light/addressable_light.h"

namespace aqifx {

inline float clampf(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }

template <typename Mapper>
struct LayoutDescriptor {
  int segs;
  int leds_per_seg;
  int height_per_seg;
  Mapper mapper;

  template <typename Callback>
  inline void for_each_pixel(int seg, int row, Callback &&cb) const {
    mapper(seg, row, std::forward<Callback>(cb));
  }
};

template <typename Mapper>
inline LayoutDescriptor<Mapper> make_layout(int segs, int leds_per_seg, int height_per_seg, Mapper mapper) {
  return LayoutDescriptor<Mapper>{segs, leds_per_seg, height_per_seg, std::move(mapper)};
}

template <typename Addressable, typename Layout>
inline void render(Addressable &it,
                   const Layout &layout,
                   const float aqi,
                   const float blinklevel,
                   float max_val_col = 150.0f,
                   float max_val_bri = 150.0f,
                   float cut_value = NAN,
                   float green_threshold = 40.0f) {
  float bri = 1.0f;
  const int height_total = layout.height_per_seg * layout.segs;

  float v_cut = std::isnan(cut_value) ? aqi : cut_value;
  v_cut = clampf(v_cut, 0.0f, max_val_bri);

  const float pct_bri = v_cut / max_val_bri;
  const float lit_f = pct_bri * static_cast<float>(height_total);
  int full_bri_leds = static_cast<int>(std::floor(lit_f));
  int lit_leds = full_bri_leds + 1;
  float semi_bri = lit_f - std::floor(lit_f);
  if (full_bri_leds < 0) {
    full_bri_leds = 0;
  }
  if (lit_leds <= 1) {
    lit_leds = 1;
    semi_bri = 1.0f;
  }

  int blink_height = 100000;
  if (blinklevel != NAN) {
    blink_height = static_cast<int>(height_total * blinklevel);
  }
  if (blink_height >= height_total) {
    blink_height = height_total - 1;
  }

  uint16_t period_mul = 4;
  uint16_t period = 256 * period_mul * 2;
  uint16_t period_half = period / 2;
  static uint16_t phase = 0;
  phase += 32;

  for (int seg = 0; seg < layout.segs; ++seg) {
    const float v_for_color = clampf(cut_value, 0.0f, max_val_col);

    bool pure_green = (v_for_color <= green_threshold);

    for (int row = 0; row < layout.height_per_seg; ++row) {
      float h = static_cast<float>(seg * layout.height_per_seg + row);
      float bf = (h < full_bri_leds) ? 1.0f : (h < lit_leds ? semi_bri : 0.0f);
      bf = clampf(bf, 0.0f, 1.0f);

      uint8_t val8 = static_cast<uint8_t>(std::round(255.0f * bf * bri));

      uint8_t hue8;
      if (pure_green) {
        // Pure green
        hue8 = static_cast<uint8_t>(std::round(120.0f * 255.0f / 360.0f));
      } else {
        // Green to red gradient
        float color_fraction = (v_for_color - green_threshold) / (max_val_col - green_threshold);
        color_fraction = clampf(color_fraction, 0.0f, 1.0f);
        float hue = 120.0f * (1.0f - color_fraction); // 120 (green) -> 0 (red)
        hue8 = static_cast<uint8_t>(std::round(hue * 255.0f / 360.0f));
      }

      ESPHSVColor hsv;
      hsv.hue = hue8;
      hsv.saturation = 255;
      hsv.value = val8;

      phase = phase % (period);

      uint16_t tri = (phase < period_half) ? static_cast<uint16_t>(phase / 2)
                                           : static_cast<uint16_t>((period - phase) / 2);

      uint8_t pulse_v = 100 + (tri / period_mul / 2);

      auto hsv_pixel = hsv;
      if (h == blink_height) {
        hsv_pixel.hue = static_cast<uint8_t>(std::round(240.0f * 255.0f / 360.0f));
        hsv_pixel.value = pulse_v;
      }

      layout.for_each_pixel(seg, row, [&](int led_index) { it[led_index] = hsv_pixel; });
    }
  }
}

}  // namespace aqifx