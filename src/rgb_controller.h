#pragma once

#include "esphome.h"
#include "ui_rgb_aqifx_profiles.h"
#include "time_helpers.h"

namespace thebox {

struct RgbConfig {
    esphome::light::LightState *rgb_profile_strip;
    esphome::light::LightState *rgb_profile_dual_strip;
    esphome::light::LightState *rgb_profile_fans;
    esphome::select::Select *cfg_rgb_mode;
    esphome::sensor::Sensor *sensor_aqi_value;
    esphome::switch_::Switch *cfg_rgb_boot_animation;
    esphome::select::Select *ui_mode_select;
    esphome::number::Number *cfg_brightness_manual;
    esphome::datetime::TimeEntity *cfg_auto_time_day;
    esphome::datetime::TimeEntity *cfg_auto_time_night;
    esphome::number::Number *cfg_brightness_auto_day;
    esphome::number::Number *cfg_brightness_auto_night;
    esphome::time::RealTimeClock *sntp_clock;
    float *rgb_current_brightness;
    bool *rgb_is_on;
};

inline RgbConfig rgb_config;

inline void apply_rgb_state() {
    using esphome::light::LightState;
    LightState *active = nullptr;

    auto selected_option = rgb_config.cfg_rgb_mode->current_option();

    if (selected_option == "Single LED Strip (30) + Clone") {
        active = rgb_config.rgb_profile_strip;
        if (rgb_config.rgb_profile_fans->get_effect_name() != "None") {
            rgb_config.rgb_profile_fans->turn_on().set_effect("none").set_brightness(0).perform();
            rgb_config.rgb_profile_fans->turn_off().set_brightness(0.0f).perform();
        }
        if (rgb_config.rgb_profile_dual_strip->get_effect_name() != "None") {
            rgb_config.rgb_profile_dual_strip->turn_on().set_effect("none").set_brightness(0).perform();
            rgb_config.rgb_profile_dual_strip->turn_off().set_brightness(0.0f).perform();
        }
    } else if (selected_option == "Dual LED Strip (2x30)") {
        active = rgb_config.rgb_profile_dual_strip;
        if (rgb_config.rgb_profile_fans->get_effect_name() != "None") {
            rgb_config.rgb_profile_fans->turn_on().set_effect("none").set_brightness(0).perform();
            rgb_config.rgb_profile_fans->turn_off().set_brightness(0.0f).perform();
        }
        if (rgb_config.rgb_profile_strip->get_effect_name() != "None") {
            rgb_config.rgb_profile_strip->turn_on().set_effect("none").set_brightness(0).perform();
            rgb_config.rgb_profile_strip->turn_off().set_brightness(0.0f).perform();
        }
    } else { // Fan modes selected
        active = rgb_config.rgb_profile_fans;
        if (rgb_config.rgb_profile_strip->get_effect_name() != "None") {
            rgb_config.rgb_profile_strip->turn_on().set_effect("none").set_brightness(0).perform();
            rgb_config.rgb_profile_strip->turn_off().set_brightness(0.0f).perform();
        }
        if (rgb_config.rgb_profile_dual_strip->get_effect_name() != "None") {
            rgb_config.rgb_profile_dual_strip->turn_on().set_effect("none").set_brightness(0).perform();
            rgb_config.rgb_profile_dual_strip->turn_off().set_brightness(0.0f).perform();
        }
    }

    if (active == nullptr) {
        *rgb_config.rgb_is_on = false;
        *rgb_config.rgb_current_brightness = 0.0f;
        return;
    }

    using std::isnan;
    bool is_warming_up = millis() < (aqifx::kWarmupDurationS * 1000);
    bool aqi_available = rgb_config.sensor_aqi_value->has_state() && !isnan(rgb_config.sensor_aqi_value->state);
    bool show_boot_animation = is_warming_up && rgb_config.cfg_rgb_boot_animation->state;
    const char *const effect_state = "AQI State";
    const char *const effect_boot = "AQI Boot Sweep";

    auto set_state = [&](float brightness, const char *effect, bool on) {
        if (on) {
            active->turn_on().set_effect(effect).set_brightness(brightness).perform();
        } else {
            active->turn_off().set_brightness(0.0f).perform();
        }
        *rgb_config.rgb_is_on = on;
        *rgb_config.rgb_current_brightness = on ? brightness : 0.0f;
    };

    float brightness;
    auto mode = rgb_config.ui_mode_select->current_option();
    if (mode == "MANUAL") {
        brightness = rgb_config.cfg_brightness_manual->state / 100.0f;
    } else if (mode == "AUTO") {
        auto t = rgb_config.sntp_clock->now();
        auto td = rgb_config.cfg_auto_time_day->state_as_esptime();
        auto tn = rgb_config.cfg_auto_time_night->state_as_esptime();
        bool day_mode = time_helpers::is_day_mode(t, td, tn);
        brightness = (day_mode ? rgb_config.cfg_brightness_auto_day->state : rgb_config.cfg_brightness_auto_night->state) / 100.0f;
    } else {
        brightness = 0.0f;
    }

    if (show_boot_animation) {
        set_state(brightness, effect_boot, true);
    } else if (aqi_available) {
        set_state(brightness, effect_state, true);
    } else {
        set_state(0.0f, effect_state, false);
    }
}

} // namespace thebox
