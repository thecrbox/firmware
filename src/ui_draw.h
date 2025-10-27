#pragma once
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <string>
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/image/image.h"
#include "esphome/components/qr_code/qr_code.h"
#include "esphome/components/wifi/wifi_component.h"

namespace ui {

// Make unqualified BaseFont visible in this namespace
    using esphome::display::BaseFont;

// ================== Layout constants (tune once here) ==================
    inline constexpr int W                 = 128;
    inline constexpr int H                 = 64;

    inline constexpr int Y_OFFSET_METRIC_TINY  = 11;
    inline constexpr int Y_OFFSET_METRIC_BARS  = 20;
    inline constexpr int Y_OFFSET_METRIC_VALUE = 32;

    inline constexpr int BAR_W             = 24;
    inline constexpr int BAR_H             = 8;

    inline constexpr int PADDING                = 2;
    inline constexpr int ICON                   = 16; // icon size (square)
    inline constexpr int Y_OFFSET_ICON_SUB_ROW1 = ICON+PADDING; // text row1 below icon
    inline constexpr int Y_OFFSET_ICON_SUB_ROW2 = 32; // text row2 below icon
    inline constexpr int X_LAST_ICON_CENTER     = W-16; // rightmost icon position center
    inline constexpr int X_OFFSET_ICONS         = 32; // px between right-aligned icon centers
    inline constexpr int Y_ICONS                = ICON+PADDING; // icon row y

    inline constexpr int HINT_IMG_Y   = 4;   // hint icon y
    inline constexpr int HINT_TEXT_Y  = 1;   // hint text y
    inline constexpr int HINT_TEXT_DX = 10;  // label x-offset from icon

    inline constexpr int HINT_X_LEFT  = 40;  // first hint x
    inline constexpr int HINT_X_RIGHT = 84;  // second hint x

// Where to print the "focused item description"
    inline constexpr int DESC_X       = 0;
    inline constexpr int DESC_Y       = Y_ICONS + Y_OFFSET_ICON_SUB_ROW2; // ~50

// ================== Small positioning helpers ==================
    inline int right_icon_x(int index_from_right) {
        return X_LAST_ICON_CENTER - index_from_right * X_OFFSET_ICONS;
    }

// ================== Tiny utility helpers (no layout change) ==================
    inline int clamp_pct(int v) { return std::max(0, std::min(100, v)); }

    inline int bars_from(float value, float step, int max_bars = 5) {
        int n = static_cast<int>(std::floor(value / step));
        if (n < 0) n = 0; if (n > max_bars) n = max_bars; return n;
    }

    inline std::string format_sensor_value(float value, int decimals = 0, const char *suffix = "") {
        if (std::isnan(value)) {
            return "?";
        }

        char buffer[32];
        if (decimals <= 0) {
            std::snprintf(buffer, sizeof(buffer), "%.0f", value);
        } else {
            std::snprintf(buffer, sizeof(buffer), "%.*f", decimals, value);
        }

        std::string result(buffer);
        if (suffix != nullptr && suffix[0] != '\0') {
            result += suffix;
        }
        return result;
    }

    inline void print_time_centered(esphome::display::Display &it, int x, int y,
                                    BaseFont *font, int hour, int minute) {
        using esphome::display::COLOR_ON;
        using esphome::display::TextAlign;
        it.printf(x, y, font, COLOR_ON, TextAlign::TOP_CENTER, "%02d:%02d", hour, minute);
    }

// 0=RGB, 1=fan, 2=time, -1=none
    inline int focus_index_from_state(const char *s) {
        if (!s) return -1;
        if (std::strcmp(s, "config_auto_night_rgb")  == 0
        || std::strcmp(s, "config_auto_day_rgb")  == 0
        || std::strcmp(s, "config_manual_rgb")  == 0
        )
            return 0;
        if (std::strcmp(s, "config_auto_night_fan")  == 0
        || std::strcmp(s, "config_auto_day_fan")  == 0
        || std::strcmp(s, "config_manual_fan")  == 0
        )
            return 1;
        if (std::strcmp(s, "config_auto_night_time") == 0
        || std::strcmp(s, "config_auto_day_time") == 0
        )
            return 2;
        return -1;
    }

// ================== Drawing primitives ==================
    inline void draw_rect_count(esphome::display::Display &it,
                                int x, int y, int w, int h, int count, int number) {
        if (count <= 0 || w <= 0 || h <= 0) return;
        if (number < 0) number = 0;
        if (number > count) number = count;

        const int spacing = 1;
        const int total_spacing = (count > 0 ? (count - 1) * spacing : 0);
        int base_w = (w - total_spacing) / count;
        if (base_w < 1) base_w = 1;

        const int used_w = base_w * count + total_spacing;
        const int leftover = w - used_w;

        int rx = x;
        for (int i = 0; i < number; i++) {
            const int rw = base_w + (i < leftover ? 1 : 0);
            it.filled_rectangle(rx, y, rw, h);
            rx += rw + spacing;
        }

        if (number < count) {
            const int remain = count - number;
            int extra_in_unfilled = 0;
            if (leftover > number) extra_in_unfilled = std::min(leftover - number, remain);
            const int unfilled_w = base_w * remain + extra_in_unfilled + spacing * (remain - 1);
            it.rectangle(rx, y, unfilled_w, h);
        }
    }

    inline void draw_icon_and_value(esphome::display::Display &it, int cx, int y,
                                esphome::display::BaseImage* img,
                                BaseFont *font,
                                int value, int value2 = -1) {
        using esphome::display::COLOR_ON;
        using esphome::display::TextAlign;

        it.image(cx, y, img, ImageAlign::TOP_CENTER, COLOR_ON);

        auto print_pct = [&](int yy, int v) {
            if (v == 100) {
                it.printf(cx, yy, font, COLOR_ON, TextAlign::TOP_CENTER, "MAX");
            } else if (v == 0) {
                it.printf(cx, yy, font, COLOR_ON, TextAlign::TOP_CENTER, "OFF");
            } else {
                it.printf(cx, yy, font, COLOR_ON, TextAlign::TOP_CENTER, "%d%%", v);
            }
        };

        print_pct(y + Y_OFFSET_ICON_SUB_ROW1, value);
        if (value2 >= 0)
            print_pct(y + Y_OFFSET_ICON_SUB_ROW2, value2);
    }

    inline void draw_icon_and_text(esphome::display::Display &it, int cx, int y,
                                    esphome::display::BaseImage* img,
                                    BaseFont *font,
                                    const char* text, const char* text2 = nullptr) {
        using esphome::display::COLOR_ON;
        using esphome::display::TextAlign;

        it.image(cx, y, img, ImageAlign::TOP_CENTER, COLOR_ON);

        it.printf(cx, y + Y_OFFSET_ICON_SUB_ROW1, font, COLOR_ON, TextAlign::TOP_CENTER, "%s", text);
        if (text2)
            it.printf(cx, y + Y_OFFSET_ICON_SUB_ROW2, font, COLOR_ON, TextAlign::TOP_CENTER, "%s", text2);
    }

    inline void draw_metric(esphome::display::Display &it,
                            int cx, int y,
                            const char *title,
                            float value,
                            int bars_filled,
                            BaseFont *font_tiny,
                            BaseFont *font_value,
                            esphome::display::BaseImage *left_icon /*=nullptr*/) {
        using esphome::display::COLOR_ON;
        using esphome::display::TextAlign;

        int text_x = cx;
        if (left_icon != nullptr) {
            it.image(cx - ICON, y, left_icon, ImageAlign::TOP_LEFT, COLOR_ON);
            text_x = cx + PADDING;
        }

        it.print(text_x, y + Y_OFFSET_METRIC_TINY, font_tiny, COLOR_ON, TextAlign::TOP_LEFT, title);
        draw_rect_count(it, cx - ICON, y + Y_OFFSET_METRIC_BARS, BAR_W, BAR_H, 5, bars_filled);
        it.printf(cx - ICON, y + Y_OFFSET_METRIC_VALUE, font_value, COLOR_ON, TextAlign::TOP_LEFT, "%.1f", value);
    }

    inline void draw_gauge_title_center(esphome::display::Display &it,
                            int cx, int ty,
                            int rout, int rin,
                            float value,
                            int max,
                            const char *title,
                            BaseFont *font_title,
                            BaseFont *font_value) {
        using esphome::display::COLOR_ON;
        using esphome::display::TextAlign;

        const bool has_value = !std::isnan(value);
        float clamped = has_value ? std::min(std::max(value, 0.0f), static_cast<float>(max)) : 0.0f;
        int progress = has_value ? static_cast<int>(std::round(clamped * 100.0f / max)) : 0;
        if (progress < 0) progress = 0;
        if (progress > 100) progress = 100;

        it.filled_gauge(cx, ty + rout, rin, rout, progress);

        if (has_value) {
            it.printf(cx, ty + rout, font_title, COLOR_ON, TextAlign::BOTTOM_CENTER, "%d", static_cast<int>(std::round(clamped)));
        } else {
            it.printf(cx, ty + rout, font_title, COLOR_ON, TextAlign::BOTTOM_CENTER, "?");
        }

        it.printf(cx, ty + rout + PADDING, font_title, COLOR_ON, TextAlign::TOP_CENTER, title);
    }

    inline void draw_gauge_title_left(esphome::display::Display &it,
                                        int cx, int ty,
                                        int rout, int rin,
                                        float value,
                                        int max,
                                        const char *title,
                                        BaseFont *font_title,
                                        BaseFont *font_value) {
        using esphome::display::COLOR_ON;
        using esphome::display::TextAlign;

        const bool has_value = !std::isnan(value);
        float clamped = has_value ? std::min(std::max(value, 0.0f), static_cast<float>(max)) : 0.0f;
        int progress = has_value ? static_cast<int>(std::round(clamped * 100.0f / max)) : 0;
        if (progress < 0) progress = 0;
        if (progress > 100) progress = 100;

        it.filled_gauge(cx, ty + rout, rin, rout, progress);

        if (has_value) {
            it.printf(cx, ty + rout, font_title, COLOR_ON, TextAlign::BOTTOM_CENTER, "%d", static_cast<int>(std::round(clamped)));
        } else {
            it.printf(cx, ty + rout, font_title, COLOR_ON, TextAlign::BOTTOM_CENTER, "?");
        }

        it.printf(cx - rout, ty + rout + PADDING, font_title, COLOR_ON, TextAlign::TOP_LEFT, title);
    }

// ================== Page headers (reusable) ==================
    inline void draw_header_2icons(esphome::display::Display &it,
                                        esphome::display::BaseImage *img_left,
                                        esphome::display::BaseImage *img_right) {
        using esphome::display::COLOR_ON;
        it.image(0, 0, img_left,   COLOR_ON);
        it.image(ICON+PADDING, 0, img_right, COLOR_ON);  // 16 + 2 offset
    }

    inline void draw_mode_header_2icons_title(esphome::display::Display &it,
                                         BaseFont *font_big,
                                         const char *title,
                                         esphome::display::BaseImage *img_left = nullptr,
                                         esphome::display::BaseImage *img_right = nullptr) {
        using esphome::display::COLOR_ON;
        it.print(ICON+PADDING, 0, font_big, COLOR_ON, title);  // exact overload: (x,y,font,Color,const char*)

        if (img_left)
            it.image(0, 0, img_left, COLOR_ON);
        if (img_right)
            it.image(W - ICON - (ICON+PADDING), 0, img_right, COLOR_ON);
    }

    inline void draw_mode_header_2icons_title_wifi(esphome::display::Display &it,
                                 BaseFont *font_big,
                                 const char *title,
                                 esphome::display::BaseImage *img_left = nullptr,
                                 esphome::display::BaseImage *img_right = nullptr) {
        draw_mode_header_2icons_title(it, font_big, title, img_left, img_right);

        if (id(mywifi).is_connected()) {
            it.image(W - ICON, 0, &id(img_wifi3), COLOR_ON);
        } else {
            it.image(W - ICON, 0, &id(img_wifi0), COLOR_ON);
        }
    }

// ================== Header hints (icons + labels) ==================
    inline void draw_hint(esphome::display::Display &it,
                          int x,
                          esphome::display::BaseImage *img,
                          BaseFont *font,
                          const char *label) {
        using esphome::display::COLOR_ON;
        using esphome::display::TextAlign;
        it.image(x, HINT_IMG_Y, img, ImageAlign::TOP_LEFT, COLOR_ON);
        it.printf(x + HINT_TEXT_DX, HINT_TEXT_Y, font, COLOR_ON, TextAlign::TOP_LEFT, "%s", label);
    }

    inline void draw_hint_right_aligned(esphome::display::Display &it,
                                        esphome::display::BaseImage *img,
                                        BaseFont *font,
                                        const char *label) {
        using esphome::display::COLOR_ON;
        using esphome::display::TextAlign;
        const int icon_x = W - ICON;
        it.image(icon_x, HINT_IMG_Y, img, ImageAlign::TOP_LEFT, COLOR_ON);
        const int text_x = icon_x - PADDING;
        it.printf(text_x, HINT_TEXT_Y, font, COLOR_ON, TextAlign::TOP_RIGHT, "%s", label);
    }

    inline void draw_hints_pair(esphome::display::Display &it,
                                esphome::display::BaseImage *img_left,  const char *lbl_left,
                                esphome::display::BaseImage *img_right, const char *lbl_right,
                                BaseFont *font) {
        draw_hint(it, HINT_X_LEFT,  img_left,  font, lbl_left);
        draw_hint(it, HINT_X_RIGHT, img_right, font, lbl_right);
    }

// ================== Focus marker + description ==================
    inline void draw_focus_marker(esphome::display::Display &it,
                                  int focus_index_from_right,  // 0=RGB, 1=fan, 2=time
                                  int marker_y,
                                  esphome::display::BaseImage *arrow_right_img) {
        if (focus_index_from_right < 0) return;
        using esphome::display::COLOR_ON;
        const int x = right_icon_x(focus_index_from_right) - 20;
        it.image(x, marker_y, arrow_right_img, COLOR_ON);
    }

    inline void draw_focus_desc_auto(esphome::display::Display &it,
                                     BaseFont *font,
                                     bool day_mode,
                                     int focus_index_from_right) {
        using esphome::display::COLOR_ON;
        using esphome::display::TextAlign;

        const char *text = nullptr;
        if (focus_index_from_right == 0) {
            text = day_mode ? "RGB brightness at DAY" : "RGB brightness at NIGHT";
        } else if (focus_index_from_right == 1) {
            text = day_mode ? "FAN limit at DAY"      : "FAN limit at NIGHT";
        } else if (focus_index_from_right == 2) {
            text = day_mode ? "DAY start time"        : "NIGHT start time";
        }
        if(text)
            it.printf(DESC_X, DESC_Y, font, COLOR_ON, TextAlign::TOP_LEFT, "%s", text);
    }

    inline void draw_focus_desc_manual(esphome::display::Display &it,
                                     BaseFont *font,
                                     int focus_index_from_right) {
        using esphome::display::COLOR_ON;
        using esphome::display::TextAlign;

        const char *text = nullptr;
        if (focus_index_from_right == 0) {
            text = "RGB brightness";
        } else if (focus_index_from_right == 1) {
            text = "FAN speed";
        }
        if (text)
            it.printf(DESC_X, DESC_Y, font, COLOR_ON, TextAlign::TOP_LEFT, "%s", text);
    }

    inline void draw_qr_code(esphome::display::Display &it,
                            esphome::qr_code::QrCode *qr_code_component,
                            const std::string &val, bool left_side = false) {
        using esphome::display::COLOR_ON;

        if (qr_code_component == nullptr) {
            return;
        }

        if (!val.empty()) {
            qr_code_component->set_value(val);
        }

        constexpr int scale = 2;
        const int module_size = static_cast<int>(qr_code_component->get_size());
        const int size = module_size * scale;
        const int x = left_side ? 0 : (it.get_width() - size);
        const int y = (it.get_height() - size);

        it.qr_code(x, y, qr_code_component, COLOR_ON, scale);
    }

    inline bool draw_focus_sensors(esphome::display::Display &it,
                                   BaseFont *font_value,
                                   BaseFont *font_small,
                                   BaseFont *font_tiny) {
        using esphome::display::COLOR_ON;
        using esphome::display::TextAlign;

        const std::string &focus = id(focus_sensors).state;

        if (focus == "aqi") {
            std::string title = "AQI: " + id(sensor_aqi_category).state;
            draw_gauge_title_left(it, X_OFFSET_ICONS, ICON,
                                  X_OFFSET_ICONS, X_OFFSET_ICONS - BAR_H,
                                  id(sensor_aqi_value).state, 500,
                                  title.c_str(),
                                  font_value, font_value);
            return true;
        }

        if (focus == "aqi2") {
            it.printf(0, Y_ICONS, font_tiny, COLOR_ON, TextAlign::TOP_LEFT, "AQI");
            std::string title = id(sensor_aqi_category).state;
            draw_gauge_title_center(it, X_OFFSET_ICONS, ICON,
                                    X_OFFSET_ICONS, X_OFFSET_ICONS - BAR_H,
                                    id(sensor_aqi_value).state, 500,
                                    title.c_str(),
                                    font_value, font_value);
            return true;
        }

        if (focus == "aqi3") {
            std::string title = "AQI:" + id(sensor_aqi_category).state;
            draw_gauge_title_left(it, BAR_W, ICON,
                                  BAR_W, BAR_W - BAR_H,
                                  id(sensor_aqi_value).state, 500,
                                  title.c_str(),
                                  font_small, font_small);
            return true;
        }

        if (focus == "aqi4") {
            std::string title = id(sensor_aqi_category).state;
            draw_gauge_title_center(it, BAR_W, ICON,
                                    BAR_W, BAR_W - BAR_H,
                                    id(sensor_aqi_value).state, 500,
                                    title.c_str(),
                                    font_small, font_small);
            return true;
        }

        if (focus == "temp_and_humidity") {
            const int x2 = right_icon_x(2);
            const int x3 = right_icon_x(3);
            std::string v1 = format_sensor_value(id(sensor_temperature_in).state, 0, "°C");
            std::string v2 = format_sensor_value(id(sensor_humidity_in).state, 0, "%");
            draw_icon_and_text(it, x2, Y_ICONS, &id(img_thermometer), font_small, v1.c_str());
            draw_icon_and_text(it, x3, Y_ICONS, &id(img_droplet),   font_small, v2.c_str());
            it.printf(x2 - X_OFFSET_ICONS/2, Y_ICONS, font_tiny, COLOR_ON, TextAlign::TOP_LEFT, "tem");
            it.printf(x3 - X_OFFSET_ICONS/2, Y_ICONS, font_tiny, COLOR_ON, TextAlign::TOP_LEFT, "hum");
            std::string title = "AQI: " + id(sensor_aqi_category).state;
            it.printf(0, H, font_value, COLOR_ON, TextAlign::BOTTOM_LEFT, "%s", title.c_str());
            return true;
        }

        if (focus == "pm_2_5") {
            const int x2 = right_icon_x(2);
            const int x3 = right_icon_x(3);
            std::string v1 = format_sensor_value(id(sensor_pm_2_5_in).state);
            std::string v2 = format_sensor_value(id(sensor_pm_10_0_in).state);
            draw_icon_and_text(it, x2, Y_ICONS, &id(img_cigarette), font_small, v1.c_str());
            draw_icon_and_text(it, x3, Y_ICONS, &id(img_wind),      font_small, v2.c_str());
            it.printf(x2 - X_OFFSET_ICONS/2, Y_ICONS, font_tiny, COLOR_ON, TextAlign::TOP_LEFT, "2.5");
            it.printf(x3 - X_OFFSET_ICONS/2, Y_ICONS, font_tiny, COLOR_ON, TextAlign::TOP_LEFT, "10");
            std::string title = "AQI: " + id(sensor_aqi_category).state;
            it.printf(0, H, font_value, COLOR_ON, TextAlign::BOTTOM_LEFT, "%s", title.c_str());
            return true;
        }

        if (focus == "nox") {
            const int x2 = right_icon_x(2);
            const int x3 = right_icon_x(3);
            std::string v1 = format_sensor_value(id(sensor_nox_in).state);
            std::string v2 = format_sensor_value(id(sensor_voc_in).state);
            draw_icon_and_text(it, x2, Y_ICONS, &id(img_lungs), font_small, v1.c_str());
            draw_icon_and_text(it, x3, Y_ICONS, &id(img_gastube), font_small, v2.c_str());
            it.printf(x2 - X_OFFSET_ICONS/2-3, Y_ICONS, font_tiny, COLOR_ON, TextAlign::TOP_LEFT, "NOX");
            it.printf(x3 - X_OFFSET_ICONS/2-1, Y_ICONS, font_tiny, COLOR_ON, TextAlign::TOP_LEFT, "VOC");
            std::string title = "AQI: " + id(sensor_aqi_category).state;
            it.printf(0, H, font_value, COLOR_ON, TextAlign::BOTTOM_LEFT, "%s", title.c_str());
            return true;
        }

        if (focus == "qr") {
            std::string http_url;
            if (esphome::wifi::global_wifi_component != nullptr) {
                auto addresses = esphome::wifi::global_wifi_component->get_ip_addresses();
                for (auto &ip : addresses) {
                    if (ip.is_set() && ip.is_ip4()) {
                        http_url = "http://" + ip.str();
                        break;
                    }
                }
            }

            if (!http_url.empty()) {
                draw_qr_code(it, &id(device_ip_qr_code), http_url, true);
            } else {
                it.printf(0, H, font_tiny, COLOR_ON, TextAlign::BOTTOM_LEFT, "no Wi-Fi");
            }
            return true;
        }

        return false;
    }

} // namespace ui
