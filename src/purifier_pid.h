#pragma once

//#include "thebox_globals.h"
//#include "thebox.h"
#include "esphome/components/number/number.h"     // For id(cfg_*), id(diag_*)
#include "esphome/components/sensor/sensor.h"     // For id(sensor_*)
#include "esphome/components/fan/fan.h"           // For id(fans_speed)
#include "esphome/components/time/real_time_clock.h"
#include "esphome/core/log.h"                     // For ESP_LOGD

#include <chrono>
#include <algorithm> // For std::clamp
#include <cmath>

class PID {
protected:
    float aqi_prev;
    float boost_derivative;
    float boost_integral;
    float boost_proportional;
    float boosted_fan_speed;
    std::chrono::steady_clock::time_point prev_calculation_timestamp;
    PID() : aqi_prev(0), boost_derivative(0), boost_integral(0), boost_proportional(0), boosted_fan_speed(0) {

    }

    bool IsWarm() {
        return millis() > (WARMUP_DURATION_S * 1000);
    }


public:
    static PID& Instance() {
        static PID instance;
        return instance;
    }

    float GetFanSpeed() {
        auto fan_mode = id(ui_mode_select).state;
        if (fan_mode == "MANUAL") {
            return ((float)id(cfg_fan_manual).state);
        } else if (fan_mode == "AUTO") {
            return boosted_fan_speed;
        } else {
            return 0;
        }
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

        float aqi_curr = id(sensor_aqi_value).state;
        float aqi_target_delta = (float)id(diag_auto_target_delta).state;
        float aqi_deadband = (float)id(cfg_aqi_deadband).state;
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

            if (aqi_target_delta > aqi_deadband) {
                // serious error, boost_integral needed
                boost_integral += time * aqi_target_delta * boost_integral_jump;
            } else {
                // small error, boost_integral should decay
                boost_integral -= time * boost_integral_decay;
            }

            // always needed, boost_proportional
            boost_proportional = aqi_target_delta * boost_proportional_jump;

            // clamp boosts
            boost_proportional = std::clamp(boost_proportional, 0.0f, 100.0f);
            boost_integral = std::clamp(boost_integral, 0.0f, 50.0f);
            boost_derivative = std::clamp(boost_derivative, 0.0f, 30.0f);
        }


        boosted_fan_speed = fan_min + boost_proportional + boost_integral + boost_derivative;
        boosted_fan_speed = std::clamp(boosted_fan_speed, 0.0f, fan_max);

        // move forward
        aqi_prev = aqi_curr;
        prev_calculation_timestamp = now;

        id(diag_fan_speed_auto).publish_state(boosted_fan_speed);
        id(diag_auto_fan_boost_derivative).publish_state(boost_derivative);
        id(diag_auto_fan_boost_integral).publish_state(boost_integral);
        id(diag_auto_fan_boost_proportional).publish_state(boost_proportional);
        id(diag_auto_prev_delta).publish_state(delta);
    }

    void Control() {
        if (!IsWarm()) {
            float fan_min = id(cfg_fan_lvl_min).state;
            ESP_LOGD("main", "Control, speed=idle (warmup)");
            id(fans_speed).speed = 30;
            if (id(fans_speed).preset_mode != id(ui_mode_select).state) {
                id(fans_speed).turn_on().set_preset_mode(id(ui_mode_select).state).perform();
            }
            return;
        }

        auto speed = GetFanSpeed();
        if (speed <= 0.0) {
            ESP_LOGD("main", "Control, speed=OFF");
            id(fans_speed).turn_off().set_speed(0).perform();
        } else {
            if (id(ui_mode_select).state == "AUTO") {
                ESP_LOGD("main", "Control, speed=%f (auto)", speed);
                id(fans_speed).speed = speed;
                id(fans_speed).turn_on().set_preset_mode("AUTO").perform();
            } else {
                ESP_LOGD("main", "Control, speed=%f (manual)", speed);
                id(fans_speed).turn_on().set_speed(speed).perform();
            }
        }
    }
};
