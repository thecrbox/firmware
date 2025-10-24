#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ssd1306_base/ssd1306_base.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
    namespace ssd1306_dump {

        class DUMPINGI2CSSD1306 : public ssd1306_base::SSD1306, public i2c::I2CDevice {
        public:
            void setup() override;
            void dump_config() override;
            void request_dump() { dump_once_ = true; }
            void set_dump_every_flush(bool v) { dump_every_flush_ = v; }
            void set_dump_invert(bool v) { dump_invert_ = v; }  // optional, flips bits in dump only


        protected:
            bool dump_every_flush_{false};
            bool dump_once_{false};
            bool dump_invert_{false};

            void command(uint8_t value) override;
            void write_display_data() override;

            enum ErrorCode { NONE = 0, COMMUNICATION_FAILED } error_code_{NONE};

            // log a small hex chunk (no allocations)
            inline void log_hex_chunk_(const uint8_t *p, size_t n) {
                // Print up to 16*2+16 chars per line; keep it tiny
                char line[16 * 2 + 16];
                size_t o = 0;
                for (size_t i = 0; i < n; i++) {
                    uint8_t b = dump_invert_ ? (uint8_t)(p[i] ^ 0xFFu) : p[i];
                    int w = snprintf(line + o, sizeof(line) - o, "%02X", b);
                    if (w <= 0) break;
                    o += (size_t) w;
                }
                ESP_LOGI("sh1106_dump", "%.*s", (int)o, line);
#if defined(ARDUINO_ARCH_ESP8266)
                delay(0);  // yield to WDT on 8266
#endif
            }

            inline void log_hex_header_footer_(bool header) {
                if (header) {
                    ESP_LOGI("sh1106_dump", "DUMP_HEX %dx%d bytes=%u",
                             this->get_width_internal(), this->get_height_internal(),
                             (unsigned)(this->get_buffer_length_()));
                } else {
                    ESP_LOGI("sh1106_dump", "DUMP_END");
                }
            }

        };

    }  // namespace ssd1306_dump
}  // namespace esphome