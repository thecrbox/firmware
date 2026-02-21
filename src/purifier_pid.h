#pragma once

#include "esphome/components/number/number.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/fan/fan.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/core/log.h"

#include <chrono>
#include <algorithm>
#include <cmath>

class PID {
protected:
    bool transition = true;

    float aqi_prev;
    float boost_derivative;
    float boost_integral;
    float boost_proportional;
    float boosted_fan_speed;
    float dynamic_integral_cap;
    std::chrono::steady_clock::time_point prev_calculation_timestamp;

    static constexpr float INTEGRAL_MIN_CAP_IN_ZONE = 5.0f;
    PID() : aqi_prev(0), boost_derivative(0), boost_integral(0), boost_proportional(0), boosted_fan_speed(0), dynamic_integral_cap(0) {

    }

    void SetTransition(bool value) {
        transition = value;
    }

public:
    static PID& Instance() {
        static PID instance;
        return instance;
    }

    bool IsTransition() {
        return transition;
    }

    bool ClearTransition() {
        bool was_transition = transition;
        transition = false;
        return was_transition;
    }

    float GetAutoFanSpeed() {
        return boosted_fan_speed > 0 ? boosted_fan_speed : 2.0f;
    }

    bool IsWarm() {
        return millis() > (WARMUP_DURATION_S * 1000);
    }

    void Calculate() {
        const auto now{std::chrono::steady_clock::now()};
        const std::chrono::duration<double> elapsed_seconds{now - prev_calculation_timestamp};
        float time = elapsed_seconds.count();

        const bool aqi_ready = id(sensor_aqi_value).has_state() && !std::isnan(id(sensor_aqi_value).state);

        if (!IsWarm()) {
            prev_calculation_timestamp = now;
            return;
        }

        if (!aqi_ready) {
            ESP_LOGD("main", "PID::Calculate skipped - AQI sensor not ready");
            prev_calculation_timestamp = now;
            return;
        }

        // limit the fans
        float fan_min = id(cfg_fan_lvl_min).state;
        float fan_max = 100;
        auto t = id(mysntp).now();
        const bool time_valid = t.is_valid();
        auto td = id(cfg_auto_time_day).state_as_esptime();
        auto tn = id(cfg_auto_time_night).state_as_esptime();
        bool day_mode = true;
        if (time_valid) {
            if (td.hour < tn.hour || (td.hour == tn.hour && td.minute < tn.minute)) {
                // normal case: day starts before night
                day_mode = (t.hour > td.hour || (t.hour == td.hour && t.minute >= td.minute)) &&
                           (t.hour < tn.hour || (t.hour == tn.hour && t.minute < tn.minute));
            } else {
                // special case: day starts after night (overnight)
                day_mode = (t.hour > td.hour || (t.hour == td.hour && t.minute >= td.minute)) ||
                           (t.hour < tn.hour || (t.hour == tn.hour && t.minute < tn.minute));
            }
        }

        if (!day_mode) {
            id(diag_auto_fan_lvl_max).publish_state(id(cfg_fan_auto_night).state);
            fan_max = id(cfg_fan_auto_night).state;
        } else {
            id(diag_auto_fan_lvl_max).publish_state(id(cfg_fan_auto_day).state);
            fan_max = id(cfg_fan_auto_day).state;
        }
        float boost_derivative_jump = id(cfg_boost_derivative_jump).state;
        float boost_derivative_decay = id(cfg_boost_derivative_decay).state;
        float boost_integral_jump = id(cfg_boost_integral_jump).state;
        float boost_integral_decay = id(cfg_boost_integral_decay).state;
        float boost_proportional_jump = id(cfg_boost_proportional_jump).state;
        float integral_max_global = id(cfg_integral_max_global).state;

        float aqi_curr = id(sensor_aqi_value).state;
        float aqi_target = id(cfg_aqi_target).state;
        float aqi_target_delta = aqi_curr - aqi_target;
        if (aqi_target_delta < 0) aqi_target_delta = 0;

        float integral_damping_zone = id(cfg_integral_damping_zone).state;
        float delta = aqi_curr - aqi_prev;

        if (!time_valid) {
            ESP_LOGD("main", "PID::Calculate running without valid time - assuming day mode");
        }

        if (IsWarm()) {

            if (delta > 0) {
                // rising, boost_derivative needed
                boost_derivative += time * delta * boost_derivative_jump;
            } else {
                // falling, boost_derivative should decay
                boost_derivative -= time * boost_derivative_decay;
            }

            // always needed, boost_proportional
            boost_proportional = aqi_target_delta * boost_proportional_jump;

            // Integral part damper: when in the damping zone
            // dynamic_integral_cap is calculated.
            // Accumulation and decay of integral term targets dynamic_integral_cap.
            // When error is out of the damping zone, dynamic cap equals integral_max_global
            // When error is in a damping zone, dynamic cap is lerped between INTEGRAL_MIN_CAP_IN_ZONE
            // and integral_max_global

            // 1. Determine the 'dynamic_integral_cap'
            if (aqi_target_delta >= integral_damping_zone) {
                dynamic_integral_cap = integral_max_global;
            } else if (aqi_target_delta > 0 && aqi_target_delta < integral_damping_zone) {
                float zone_pos = aqi_target_delta / integral_damping_zone;
                dynamic_integral_cap = INTEGRAL_MIN_CAP_IN_ZONE +
                                       (integral_max_global - INTEGRAL_MIN_CAP_IN_ZONE) * zone_pos;
                dynamic_integral_cap = std::clamp(dynamic_integral_cap, INTEGRAL_MIN_CAP_IN_ZONE, integral_max_global);
            } else { // aqi_target_delta <= 0
                dynamic_integral_cap = 0.0f; // Ensure it decays fully towards 0
            }
            dynamic_integral_cap = std::max(0.0f, dynamic_integral_cap); // Ensure non-negative

            // 2. Update 'boost_integral' with controlled accumulation and decay
            float integral_rate_of_change = 0.0f;
            if (aqi_target_delta > 0) {
                if (boost_integral < dynamic_integral_cap) {
                    integral_rate_of_change = time * aqi_target_delta * boost_integral_jump;
                } else {
                    // When above the dynamic cap do not accumulate; start decaying
                    integral_rate_of_change = -time * boost_integral_decay;
                }
            } else { // aqi_target_delta <= 0
                // When below the target, start decaying
                integral_rate_of_change = -time * boost_integral_decay;
            }
            boost_integral += integral_rate_of_change;

            // 3. Sanitize 'boost_integral' (above 0)
            boost_integral = std::max(0.0f, boost_integral);

            // clamp boosts (for P and D)
            boost_proportional = std::clamp(boost_proportional, 0.0f, 100.0f);
            boost_derivative = std::clamp(boost_derivative, 0.0f, 30.0f);
        }

        boosted_fan_speed = fan_min + boost_proportional + boost_integral + boost_derivative;
        boosted_fan_speed = std::clamp(boosted_fan_speed, 0.0f, fan_max);

        // move forward
        aqi_prev = aqi_curr;
        prev_calculation_timestamp = now;

        id(diag_dynamic_integral_cap).publish_state(dynamic_integral_cap);
        id(diag_fan_speed_auto).publish_state(boosted_fan_speed);
        id(diag_boost_derivative).publish_state(boost_derivative);
        id(diag_boost_integral).publish_state(boost_integral);
        id(diag_boost_proportional).publish_state(boost_proportional);
        id(diag_auto_prev_delta).publish_state(delta);
    }

    
};
