#pragma once

#include <cmath>
#include <algorithm>
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace aqi_calculator {

struct Breakpoint { float Clow; float Chigh; int Ilow; int Ihigh; };

// EPA PM2.5 AQI (truncate to 0.1 µg/m³)
inline float aqi_pm25(float c) {
    if (std::isnan(c)) return NAN;
    float Ct = floorf(c * 10.0f) / 10.0f;
    const Breakpoint bp[] = {
        {  0.0f,  12.0f,   0,  50},
        { 12.1f,  35.4f,  51, 100},
        { 35.5f,  55.4f, 101, 150},
        { 55.5f, 150.4f, 151, 200},
        {150.5f, 250.4f, 201, 300},
        {250.5f, 350.4f, 301, 400},
        {350.5f, 500.4f, 401, 500},
    };
    for (const auto &b : bp) {
        if (Ct <= b.Chigh)
            return (b.Ihigh - b.Ilow) * (Ct - b.Clow) / (b.Chigh - b.Clow) + b.Ilow;
    }
    return 500.0f;
}

// EPA PM10 AQI (truncate to integer µg/m³)
inline float aqi_pm10(float c) {
    if (std::isnan(c)) return NAN;
    float Ct = floorf(c);
    const Breakpoint bp[] = {
        {  0.0f,  54.0f,   0,  50},
        { 55.0f, 154.0f,  51, 100},
        {155.0f, 254.0f, 101, 150},
        {255.0f, 354.0f, 151, 200},
        {355.0f, 424.0f, 201, 300},
        {425.0f, 504.0f, 301, 400},
        {505.0f, 604.0f, 401, 500},
    };
    for (const auto &b : bp) {
        if (Ct <= b.Chigh)
            return (b.Ihigh - b.Ilow) * (Ct - b.Clow) / (b.Chigh - b.Clow) + b.Ilow;
    }
    return 500.0f;
}

inline float calculate_aqi(float pm1, float pm25, float pm4, float pm10) {
    float combined = -1.0f;

    if (!std::isnan(pm1))  { float v = aqi_pm25(pm1);  if (!std::isnan(v) && v > combined) combined = v; }
    if (!std::isnan(pm25)) { float v = aqi_pm25(pm25); if (!std::isnan(v) && v > combined) combined = v; }
    if (!std::isnan(pm4))  { float v = aqi_pm10(pm4);  if (!std::isnan(v) && v > combined) combined = v; }
    if (!std::isnan(pm10)) { float v = aqi_pm10(pm10); if (!std::isnan(v) && v > combined) combined = v; }

    if (combined < 0.0f) return NAN;

    return std::clamp(combined, 0.0f, 500.0f);
}

} // namespace aqi_calculator
