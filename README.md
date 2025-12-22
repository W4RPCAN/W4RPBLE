# W4RPBLE: Automotive automation modules for ESP32

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.2.0-purple.svg)]()

W4RPBLE is a specialized C++ library designed to transform Espressif ESP32 microcontrollers into high-performance automotive modules. It functions as a bidirectional bridge between a vehicle's Controller Area Network (CAN) bus and a mobile device via Bluetooth Low Energy (BLE).

Unlike general-purpose Arduino CAN libraries, W4RPBLE is engineered specifically for the constraints of the automotive environment. It implements an autonomous Rules Engine that executes logic locally on the device (ensuring safety even if the phone disconnects), a high-throughput O(1) signal processing architecture, and a Differential OTA update system designed for low-bandwidth connections.

## Key Features

*   **High-Performance CAN**:
    *   **O(1) Signal Lookup**: Uses a hash map to process CAN frames in constant time, regardless of signal count.
    *   **Burst Draining**: Capable of handling 2000+ frames per second without buffer overflows.
    *   **Listen-Only Mode**: Allows safe sniffing of bus traffic without risking error frames.

*   **Robust BLE Bridging**:
    *   **Streaming Protocol**: Implements a custom chunking algorithm (180-byte payloads) to reliably transfer large JSON rulesets and firmware updates over standard BLE characteristics.
    *   **Zero Dependency**: The module operates autonomously. The phone is treated as a transient UI, not a logic controller.

*   **Embedded Rules Engine**:
    *   **Safe Execution**: Logic is defined in a static JSON format, preventing runtime scripting errors.
    *   **Recursion Guard**: DFS traversal with strict depth limits prevents stack overflows.
    *   **Debounce Logic**: Integrated hold-timers filter out noise from automotive sensors.

*   **Differential OTA Updates**:
    *   **Bandwidth Efficient**: Uses the Janpatch algorithm to apply small binary patches (approx 50KB) instead of full firmware images (1MB), reducing update time by 90%.
    *   **Dual-Partitions**: Use of ESP32 OTA partitions ensures safe rollbacks if an update fails.

## Quick Start

### Installation

1.  Download this repository as a ZIP.
2.  Install in Arduino IDE via **Sketch -> Include Library -> Add .ZIP Library**.
3.  Include `<W4RPBLE.h>` in your project.

### Minimal Example

```cpp
#include <W4RPBLE.h>

W4RPBLE w4rp;

void setup() {
  Serial.begin(115200);

  // Initialize the specific hardware bridge
  w4rp.setCanMode(W4RPBLE::CanMode::LISTEN_ONLY);
  w4rp.begin();
}

void loop() {
  // Main task runner
  w4rp.loop();
}
```

## Usage

### 1. Configuration Methods

These methods must be called inside `setup()` **before** calling `w4rp.begin()`.

```cpp
// 1. Identity Configuration
w4rp.setBleName("Exhaust Controller");      // Name in BLE Scan
w4rp.setModuleFirmware("1.0.0");            // Version reported to App
w4rp.setModuleHardware("esp32c3-mini-v1");  // Hardware ID for OTA
w4rp.setModuleSerial("EXHST-001");          // Serial Number

// 2. Hardware Configuration
w4rp.setPins(21, 20, 8); // TX, RX, LED
w4rp.setCanMode(W4RPBLE::CanMode::LISTEN_ONLY);
```

### 2. Registering Capabilities

Capabilities are the actions your module can perform (e.g., "Open Switch", "Set PWM"). You register them using C++ Lambda functions.

**Basic On/Off Action (Relay)**

```cpp
w4rp.registerCapability("relay_1", [](const W4RPBLE::ParamMap &params) {
    if (params.find("state") == params.end()) return;

    // Parse logic from the Rules Engine
    int state = params.at("state").toInt();
    
    // Execute Hardware
    digitalWrite(RELAY_PIN, state ? HIGH : LOW);
});
```

**Complex Action (PWM with UI Metadata)**

If you want the App to render a specific UI (like a Slider), you must pass a `CapabilityMeta` struct.

```cpp
W4RPBLE::CapabilityMeta meta;
meta.id = "valve_pwm";
meta.label = "Exhaust Valve";
meta.category = "output";

// Define a 0-100 Integer Slider
meta.params.push_back({
    .name = "duty",
    .type = "int",
    .min = 0,
    .max = 100
});

w4rp.registerCapability(meta, [](const W4RPBLE::ParamMap &params) {
    int duty = params.at("duty").toInt();
    // Clamp for safety
    if (duty < 0) duty = 0;
    if (duty > 100) duty = 100;
    
    ledcWrite(PWM_CHANNEL, duty);
});
```

## API Reference

The following is a comprehensive list of all public methods exposed by the `w4rp` instance.

| Method | Description |
|---|---|
| **Lifecycle** | |
| `begin()` | Initializes BLE, CAN, NVS, and Allocators. Must be called last in setup. |
| `loop()` | Main processing task. Must be called in the Arduino loop. |
| **Configuration** | |
| `setBleName(name)` | Sets the local device name advertised over BLE. |
| `setModuleFirmware(fw)` | Sets the firmware version string (e.g., "1.0.0"). |
| `setModuleHardware(hw)` | Sets the hardware model ID (used for OTA compatibility checks). |
| `setModuleSerial(sn)` | Sets the device serial number. |
| `setModuleIdOverride(id)`| Overrides the default `W4RP-MAC` unique identifier. |
| `setCanMode(mode)` | Sets driver mode: `NORMAL` (ACK active), `LISTEN_ONLY` (Safe), or `NO_ACK`. |
| `setPins(tx, rx, led)` | Overrides the default GPIO pins for CAN TX, RX, and Status LED. |
| `setCanTiming(config)` | Overrides the default 500kbps timing with a custom `twai_timing_config_t`. |
| **Capabilities** | |
| `registerCapability(id, fn)` | Registers a capability with no UI metadata (hidden from manual control). |
| `registerCapability(meta, fn)` | Registers a capability with UI metadata (Slider, Toggle, etc.). |
| **Storage** | |
| `nvsWrite(key, val)` | Writes a string to the `w4rp` NVS namespace. |
| `nvsRead(key)` | Reads a string from the `w4rp` NVS namespace. |
| **Information** | |
| `getModuleId()` | Returns the current unique module ID. |
| `getFwVersion()` | Returns the current firmware version. |

## Documentation

We have detailed guides for both firmware and application developers.

### Introduction
*   [Why BLE vs WiFi?](docs/introduction/why-ble-vs-wifi.md): Technical comparison for automotive use cases.
*   [Ecosystem Philosophy](docs/introduction/ecosystem-philosophy.md): Open vs Closed architecture design.

### Firmware Development
*   [Getting Started](docs/guides-firmware/getting-started.md): Detailed setup guide.
*   [Creating Capabilities](docs/guides-firmware/creating-capabilities.md): Mastering the Lambda callbacks.
*   [Safety Mechanisms](docs/guides-firmware/safety-mechanisms.md): Watchdogs and Fail-Safes.
*   [Handling Persistence](docs/guides-firmware/handling-persistence.md): Saving user data safely.

### App Development
*   [Protocol Specification](docs/guides-app-dev/protocol-specification.md): Complete BLE UUID and Packet guide.
*   [Connecting & Handshake](docs/guides-app-dev/connecting-handshake.md): How to assume control of a module.
*   [Delta OTA Integration](docs/guides-app-dev/delta-ota-integration.md): Implementing the client-side updater.

### Deep Dives
*   [Performance Limits](docs/deep-dives/performance-limits.md): Throughput analysis.
*   [Memory Architecture](docs/deep-dives/memory-architecture.md): Heap usage on ESP32-C3.
*   [OTA Internals](docs/deep-dives/ota-internals.md): Partitions and Patching logic.
*   [CRC Integrity](docs/deep-dives/crc-integrity.md): Data safety protocols.

## License

MIT License

Copyright (c) 2024 W4RP Automotive

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.