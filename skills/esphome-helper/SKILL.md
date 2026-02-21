---
name: esphome-helper
description: Provides guidance on using ESPHome commands for this project, including compile, config, and run. Use this skill when performing ESPHome-related tasks for the project.
---

# ESPHome Project Commands

This skill provides specific instructions and usage examples for common ESPHome commands within this project's context.

### Configuration Files Overview
The project now uses a base configuration file (`thebox_base.yaml`) which is included by device-specific wrapper files (e.g., `thebox_fake.yaml`, `thebox_real.yaml`). These wrapper files define device-specific substitutions like `device_name` and `sensor_type`.

To compile, configure, or run for a specific device, use its corresponding wrapper file.

### `esphome compile`
Use this command often to check your intermediate work results. It compiles the firmware for your ESPHome device.

**Usage:**
```bash
esphome compile ./<DEVICE_CONFIG_FILE>.yaml
```
**Example:**
```bash
esphome compile ./thebox_fake.yaml
```

### `esphome config`
Use this command even more often to check intermediate work results and validate your configuration.

**Usage:**
```bash
esphome config ./<DEVICE_CONFIG_FILE>.yaml
```
**Example:**
```bash
esphome config ./thebox_real.yaml
```

### `esphome run`
Use this command to flash a real device. Note that logs will become visible 20 seconds after flashing, and the whole compilation and flashing process takes at least 1 minute.

**Usage:**
```bash
esphome run ./<DEVICE_CONFIG_FILE>.yaml --device <DEVICE_NAME>.local
```
**Example:**
```bash
esphome run ./thebox_real.yaml --device box2-0003.local
```

### Troubleshooting PlatformIO
When compiling, PlatformIO may need to download ESP-IDF toolchains. Behind a proxy, this can require patience; rerun the command if transient HTTP 403 errors appear.
