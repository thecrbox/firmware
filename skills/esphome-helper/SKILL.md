---
name: esphome-helper
description: Provides guidance on using ESPHome commands for this project, including compile, config, and run. Use this skill when performing ESPHome-related tasks for the project.
---

# ESPHome Project Commands

This skill provides specific instructions and usage examples for common ESPHome commands within this project's context.

### `esphome compile`
Use this command often to check your intermediate work results. It compiles the firmware for your ESPHome device.

**Usage:**
```bash
esphome compile ./thebox4.yaml
```

### `esphome config`
Use this command even more often to check intermediate work results and validate your configuration.

**Usage:**
```bash
esphome config ./thebox4.yaml
```

### `esphome run`
Use this command to flash a real device. Note that logs will become visible 20 seconds after flashing, and the whole compilation and flashing process takes at least 1 minute.

**Usage:**
```bash
esphome run ./thebox4.yaml --device box2-0004.local
```

### Troubleshooting PlatformIO
When compiling, PlatformIO may need to download ESP-IDF toolchains. Behind a proxy, this can require patience; rerun the command if transient HTTP 403 errors appear.
