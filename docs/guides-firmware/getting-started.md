# Getting Started (Firmware)

This guide walks you through creating a new W4RPBLE module from scratch.
Use `boilerplate.ino` as your starting point.

## 1. Include and Instantiate

```cpp
#include <W4RPBLE.h>
W4RPBLE w4rp;
```
Create a global instance. Do not create it inside `setup()` or it will be destroyed when `setup` exits.

## 2. The Setup Sequence

The order of operations is critical.

### Phase A: Hardware Init
Initialize your pins (Relays, LEDs) **before** anything else.
```cpp
pinMode(RELAY_PIN, OUTPUT);
digitalWrite(RELAY_PIN, LOW); // Start safe
```

### Phase B: Module Identity
Tell the app who you are.
```cpp
w4rp.setBleName("My Module");
w4rp.setModuleFirmware("1.0.0");
```
*   **Why?**: This data is cached in the "Module Profile" sent to the app on connection.

### Phase C: W4RP Configuration
Configure the library internals.
```cpp
w4rp.setCanMode(W4RPBLE::CanMode::LISTEN_ONLY); // Always Safe Mode first!
```
*   **Crucial**: You **must** call these before `begin()`. Once `begin()` runs, the hardware drivers are locked.

### Phase D: Begin
Start the engine.
```cpp
w4rp.begin();
```
*   Allocates buffers.
*   Starts BLE Advertising.
*   Starts CAN Driver.

### Phase E: Register Capabilities
Link the text-based rules to your C++ code.
```cpp
setupRelayCapability();
```

## 3. The Loop
Keep your `loop()` clean.
```cpp
void loop() {
  w4rp.loop();
}
```
*   **Blocking Warning**: Do NOT use `delay(1000)`. If you block the loop, the CAN buffer will overflow and you will lose data.
*   **Timers**: Use non-blocking `millis()` logic if you need to do other things.
