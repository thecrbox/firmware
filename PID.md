# Purifier AUTO Mode - PID Controller Tuning

The AUTO mode for the purifier is controlled by a sophisticated, non-classical PID (Proportional-Integral-Derivative) controller. This controller automatically adjusts the fan speed based on sensor readings to reach and maintain your desired air quality level.

While the default settings are designed to be effective for most environments, you can fine-tune its behavior using the settings below.

---

### Basic Configuration

These are the most common settings you might want to adjust.

- **AUTO: Fan min**
  - **Entity ID:** `number.cfg_fan_lvl_min`
  - **Description:** The minimum speed the fan will run at in AUTO mode, even if the air quality is perfect. This ensures a constant level of air circulation.
  - **Default:** `20%`

- **AUTO: Fan max DAY / NIGHT**
  - **Entity IDs:** `number.cfg_fan_auto_day`, `number.cfg_fan_auto_night`
  - **Description:** The maximum speed the fan is allowed to reach in AUTO mode. You can set different limits for day and night to keep the purifier quiet while you sleep.
  - **Default:** Day `100%`, Night `50%`

- **AUTO: AQI target**
  - **Entity ID:** `number.cfg_aqi_target`
  - **Description:** The desired air quality index (AQI) you want the purifier to maintain. The controller will work to bring the current AQI down to this value.
  - **Default:** `25`

- **AUTO: Target Comfort Margin**
  - **Entity ID:** `number.cfg_integral_damping_zone`
  - **Description:** This setting defines a margin (in AQI points) above your AQI target. When the current AQI is within this margin, the controller's long-term corrective action (the Integral part) is scaled back. This prevents the fan from getting "overly excited" trying to fight small, persistent pollution levels it cannot easily overcome, leading to a smoother, more comfortable fan behavior when approaching the target.
  - **Default:** `30`

---

### Advanced Configuration (PID Internals)

**Warning:** Adjusting these values requires an understanding of PID control theory. Incorrect values can lead to unstable or ineffective fan behavior. These settings are hidden by default and require unlocking the "Unlock advanced settings" switch.

The final fan speed in AUTO mode is roughly `Fan min + P + I + D`. The terms are calculated as follows:

- **Proportional (P) term (`sensor.diag_boost_proportional`):** This is the primary driver, providing an immediate and strong response directly proportional to the current difference between the live AQI and the target AQI.
  - **AUTO: Coef proportional** (`number.cfg_boost_proportional_jump`): Determines how strongly the P term reacts to the current AQI delta.

- **Derivative (D) term (`sensor.diag_boost_derivative`):** This term reacts to the *rate of change* of the AQI. It provides a quick boost to the fan speed when pollution suddenly spikes and helps prevent overshooting the target.
  - **AUTO: Coef derivative** (`number.cfg_boost_derivative_jump`): The strength of the boost when AQI starts to rise.
  - **AUTO: Decay derivative %/s** (`number.cfg_boost_derivative_decay`): The speed at which the D term's boost fades.

- **Integral (I) term (`sensor.diag_boost_integral`):** This term addresses small, persistent errors over time by accumulating a boost when the AQI is consistently above the target. Its primary function is to prevent excessively high fan speeds for small, persistent errors when the system is operating within the "Target Comfort Margin." Its maximum value is dynamically capped.
  - **`sensor.diag_dynamic_integral_cap`**: This shows the current maximum value the Integral term can reach.
    - When the AQI is far from the target, this cap is high, allowing the Integral term to contribute significantly.
    - As the AQI gets closer to the target (within the "Target Comfort Margin"), this cap is progressively lowered. This specifically prevents the Integral term from accumulating too much and driving fan speeds unnecessarily high for minor, persistent pollution levels within the comfort zone.
  - **AUTO: Max Integral global cap**
    - **Entity ID:** `number.cfg_integral_max_global`
    - **Description:** This setting defines the absolute maximum value (as a percentage of fan speed) the Integral term is allowed to contribute. It can be set from `15%` to `60%`.
    - **Default:** `45%`
  - **AUTO: Coef integral** (`number.cfg_boost_integral_jump`): The speed at which the I term accumulates when the AQI is above the target and below the dynamic cap.
  - **AUTO: Decay integral %/s** (`number.cfg_boost_integral_decay`): The speed at which the I term shrinks when the AQI is at or below the target, or when the I term has reached or exceeded its dynamic cap.
