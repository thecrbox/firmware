#pragma once
#include "esphome.h"
#include <cmath>
#include "esphome/components/light/addressable_light.h"

using namespace esphome;

namespace pm25fx {

inline float clampf(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }

/*
 * EXACT rendering of your original effect, but factored.
 *
 * - segs, leds_per_seg  : layout (e.g. 4, 8)
 * - pm25_per_seg        : PM2.5 per segment (you can pass the same value to all)
 * - max_val_col/bri     : same meaning as original (40/40)
 * - cut_value           : PM2.5 used to compute the global brightness "cut"
 *                         (to be EXACT, pass the same v you use everywhere)
 * - green_threshold     : 5.0 like your code
 */
template <typename Addressable>
inline void render(Addressable &it,
                   int segs,
                   int leds_per_seg,
                   const float pm25,
                   const float blinklevel,
                   float max_val_col = 50.0f,
                   float max_val_bri = 50.0f,
                   float cut_value = NAN,
                   float green_threshold = 5.0f) {
  float bri = 1.0f;
  const int height_per_seg = 30;
  const int height_total   = height_per_seg * segs;

  float v_cut = std::isnan(cut_value) ? pm25 : cut_value;
  v_cut = clampf(v_cut, 0.0f, max_val_bri);

  const float pct_bri = v_cut / max_val_bri;
  const float lit_f   = pct_bri * (float)height_total;
  int   full_bri_leds = (int)floorf(lit_f);
  int   lit_leds      = full_bri_leds + 1;
  float semi_bri      = lit_f - floorf(lit_f);
  if (full_bri_leds < 0) {
	  full_bri_leds = 0;
  }
  if (lit_leds <= 1) {
    lit_leds = 1; semi_bri = 1.0f;
  }

  int blink_height = 100000;
  if (blinklevel != NAN) {
	blink_height = height_total * blinklevel;
  }
  if (blink_height >= height_total) {
    blink_height = height_total - 1;
  }

  //ESP_LOGD("main", "total_h=%d blink_h=%d", height_total, blink_height);


  uint16_t period_mul = 4;
  uint16_t period = 256 * period_mul * 2;
  uint16_t period_half = period/2;
  static uint16_t phase = 0;
  phase += 32;

  for (int seg = 0; seg < segs; ++seg) {
    auto v_seg = clampf(pm25, 0.0f, max_val_col);

    const float v_for_color = clampf(cut_value, 0.0f, max_val_col);
    const float pct_col     = v_for_color / max_val_col;

    // decide "green"
    bool green = (v_for_color <= green_threshold);

    for (int i = 0; i < height_per_seg; ++i) {
      float h = seg * height_per_seg + i;
      float bf = (h < full_bri_leds) ? 1.0f : (h < lit_leds ? semi_bri : 0.0f);
      bf = clampf(bf, 0.0f, 1.0f);

      uint8_t val8 = (uint8_t)roundf(255.0f * bf * bri);

      uint8_t hue8;
      if (green) {
        hue8 = (uint8_t)roundf(120.0f * 255.0f / 360.0f);
      } else {
        hue8 = (uint8_t)roundf((1.0f - pct_col) * 110.0f * 255.0f / 360.0f);
      }

      ESPHSVColor hsv;
      hsv.hue = hue8;
      hsv.saturation = 255;
      hsv.value = val8;

      // simple pulsing (triangle)
      phase = phase % (period);

      // triangle wave 0..1024..0
      uint16_t tri = (phase < period_half) 
                      ? (uint16_t)(phase / 2)
                      : (uint16_t)((period - phase) / 2);

      uint8_t pulse_v = 100 + (tri / period_mul / 2);

      const int base = seg * leds_per_seg;

      auto pos = i;
      if (h == blink_height) {
        hsv.hue = (uint8_t)roundf(240.0f * 255.0f / 360.0f);;
        hsv.value = pulse_v;
      }
      it[base + pos] = hsv;
    }
  }
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

    pm25fx::render(it, kSegs, kLedsPerSeg,
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

    pm25fx::render(it, kSegs, kLedsPerSeg,
                   value, fanlevel,
                    kMaxCol, kMaxBri,
                    /*cut_value*/ value,
                    /*green_threshold*/ kGreenThresh);
    it.schedule_show();
}

} // namespace thebox_light_effects
