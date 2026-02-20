#pragma once

#include "esphome.h"
#include "purifier_pid.h"

namespace thebox {

class StateController {
private:
    // This is a crude way to prevent recursion.
    bool is_syncing_ = false;

    struct AutoLock {
        StateController &parent_;
        AutoLock(StateController &parent) : parent_(parent) { parent_.is_syncing_ = true; }
        ~AutoLock() { parent_.is_syncing_ = false; }
    };

    StateController() = default;

public:
    static StateController& get() {
        static StateController instance;
        return instance;
    }

    StateController(const StateController&) = delete;
    StateController& operator=(const StateController&) = delete;

    // Called when the fan component is changed from HA/API
    void on_fan_update() {
        if (is_syncing_) return;
        AutoLock lock(*this);
        ESP_LOGI("state", "on_fan_update triggered");

        auto &fan = id(fans_speed);
        auto current_mode = id(ui_mode_select).current_option();

        if (!fan.state) { // Fan is off
            if (current_mode != "OFF") {
                ESP_LOGI("state", "Fan turned off, setting mode to OFF");
                id(ui_mode_select).publish_state("OFF");
            }
        } else { // Fan is on
            auto preset = fan.get_preset_mode();
            if (preset == "AUTO") {
                if (current_mode != "AUTO") {
                    ESP_LOGI("state", "Fan preset is AUTO, setting mode to AUTO");
                    id(ui_mode_select).publish_state("AUTO");
                }
            } else { // Manual mode (no preset or other preset)
                if (current_mode != "MANUAL") {
                    ESP_LOGI("state", "Fan is on without AUTO preset, setting mode to MANUAL");
                    id(ui_mode_select).publish_state("MANUAL");
                }
                // Sync speed to manual config
                if (id(cfg_fan_manual).state != fan.speed) {
                    ESP_LOGI("state", "Syncing fan speed (%.0f) to manual config", fan.speed);
                    id(cfg_fan_manual).publish_state(fan.speed);
                }
            }
        }
    }

    // Called when the UI mode select is changed (either by UI or by on_fan_update)
    void on_mode_update(const std::string &new_mode) {
        if (is_syncing_) return;
        AutoLock lock(*this);
        ESP_LOGI("state", "on_mode_update triggered with mode %s", new_mode.c_str());

        auto &fan = id(fans_speed);
        
        if (new_mode == "OFF") {
            if (fan.state) {
                ESP_LOGI("state", "Mode is OFF, turning fan off");
                fan.turn_off().perform();
            }
        } else if (new_mode == "AUTO") {
            auto preset = fan.get_preset_mode();
            if (!fan.state || preset != "AUTO") {
                ESP_LOGI("state", "Mode is AUTO, turning fan on with AUTO preset");
                fan.turn_on().set_preset_mode("AUTO").perform();
            }
        } else if (new_mode == "MANUAL") {
            float manual_speed = id(cfg_fan_manual).state;
            auto preset = fan.get_preset_mode();
            if (!fan.state || fan.speed != manual_speed || preset == "AUTO") {
                ESP_LOGI("state", "Mode is MANUAL, setting fan speed to %.0f", manual_speed);
                // Setting speed clears the preset
                fan.turn_on().set_speed(manual_speed).perform();
            }
        }
    }

    // Called when the manual fan speed number is changed from HA/UI
    void on_manual_speed_update(float new_speed) {
        if (is_syncing_) return;
        AutoLock lock(*this);
        ESP_LOGI("state", "on_manual_speed_update triggered with speed %.0f", new_speed);
        
        // If the user changes the manual speed, we should switch to MANUAL mode.
        if (id(ui_mode_select).current_option() != "MANUAL") {
            id(ui_mode_select).publish_state("MANUAL");
        } else {
            // Already in manual mode, just update the fan
            auto &fan = id(fans_speed);
            if (!fan.state || fan.speed != new_speed) {
                fan.turn_on().set_speed(new_speed).perform();
            }
        }
    }

    void on_pid_update() {
        if (is_syncing_) return;
        AutoLock lock(*this);

        if (id(ui_mode_select).current_option() == "AUTO") {
            float auto_speed = PID::Instance().GetAutoFanSpeed();
            auto &fan = id(fans_speed);
            if (fan.speed != floor(auto_speed)) {
                ESP_LOGI("state", "PID updating AUTO fan speed to %.0f", auto_speed);
                fan.speed = auto_speed;
                fan.turn_on().set_preset_mode("AUTO").perform();
            }
        }
    }
};

} // namespace thebox
