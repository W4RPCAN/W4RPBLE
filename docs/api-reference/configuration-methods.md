# Configuration Methods

These methods allow you to configure the module before initialization.
> **Important:** All configuration methods must be called **before** `w4rp.begin()`.

## Module Identity

### `setBleName(name)`
Set the name advertised over Bluetooth.
```cpp
w4rp.setBleName("Exhaust Valve");
```

### `setModuleHardware(hw)`
Set the hardware model identifier reported to the app.
```cpp
w4rp.setModuleHardware("esp32c3-mini-1");
```

### `setModuleFirmware(fw)`
Set the firmware version string.
```cpp
w4rp.setModuleFirmware("0.5.0");
```

### `setModuleSerial(serial)`
Set the device serial number.
```cpp
w4rp.setModuleSerial("UNIT-001");
```

### `setModuleIdOverride(id)`
Override the unique ID (defaults to `W4RP-<MAC>`).
```cpp
w4rp.setModuleIdOverride("W4RP-CUSTOM");
```

## Hardware Configuration

### `setPins(can_tx, can_rx, led)`
Override default GPIO pins.
```cpp
// Example for ESP32-C3 defaults
w4rp.setPins(21, 20, 8);
```

### `setCanTiming(config)`
Override CAN bus timing (default is 500kbps).
```cpp
twai_timing_config_t timing = TWAI_TIMING_CONFIG_250KBITS();
w4rp.setCanTiming(timing);
```

### `setCanMode(mode)`
Set the CAN driver operating mode.
```cpp
w4rp.setCanMode(W4RPBLE::CanMode::LISTEN_ONLY);
```
See [CAN Filtering](../advanced/can-filtering.md) for more details.
