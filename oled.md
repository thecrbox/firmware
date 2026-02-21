# OLED Display Dumping Instructions

This document describes how to access the current content of the ESPHome device's OLED display as a hexadecimal dump and convert it into a PNG image. This feature is only available on the `fake` device (`thebox_fake.yaml`).

## Configuration

The `fake` device is configured to use the `sh1306_dump` component, which can dump the display buffer to a text sensor. The dump is triggered manually.

- **`src/components/sh1306_dump/`**: This component extends the standard SSD1306 driver to allow dumping the display content.
- **`thebox_fake.yaml`**: A button with the name "Display Dump Trigger" is defined here. Pressing this button in the web interface will trigger a dump of the display content. The `display` is configured to use the `sh1306_dump` platform and sends its data to the `display_hex_dump` text sensor.

## Accessing the Display Dump

First, trigger a dump by navigating to the device's web interface (`http://box2-0004.local`) and pressing the "Display Dump Trigger" button under the "System" section.

Then, to retrieve the current state of the display as a PNG image, execute the following command in your terminal:

```bash
curl -s http://box2-0004.local/text_sensor/display_hex_dump | jq -r '.value' | python3 dump-to-png.py > oled.png
```

This command performs the following actions:
1.  **`curl -s http://box2-0004.local/text_sensor/display_hex_dump`**: Fetches the JSON output from the ESPHome device's web server. This JSON contains the latest hexadecimal representation of the display buffer. The `-s` flag silences progress output.
2.  **`jq -r '.value'`**: Parses the JSON response and extracts the raw hexadecimal string associated with the `.value` field. The `-r` flag outputs the raw string without JSON escaping.
3.  **`python3 dump-to-png.py > oled.png`**: Pipes this raw hexadecimal string to the `dump-to-png.py` script. This script converts the hex data into a monochrome PNG image, which is then saved as `oled.png` in your current directory.

After running the command, an `oled.png` file will be created, showing the current content of your ESPHome device's display.
