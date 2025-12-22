# Deep Dive: Memory Architecture

The constraints of the ESP32 (specifically the C3) define our strict memory policies.

## The Budget (ESP32-C3)

*   **Total SRAM**: 400KB
*   **WiFi/BLE Stack**: ~110KB (Static)
*   **FreeRTOS/Heap**: ~150KB (Reserved)
*   **Available for App**: ~140KB

This might seem like a lot, but fragmentation is the enemy.

## 1. The Ring Buffer (8KB Static)
For OTA, we cannot load the 50KB patch into RAM.
We use an 8KB `rx_ring_buffer`.
*   **Why 8KB?**:
    *   It's large enough to absorb BLE bursts (packets arrive faster than Flash writes during erase cycles).
    *   It's small enough to leave room for the application.
*   **Implementation**: A raw `uint8_t` array. No dynamic allocation per packet.

## 2. Dynamic JSON (Heap)
The Rules Engine is fully dynamic.
*   **Allocator**: `DynamicJsonDocument(16384)`.
*   **Why 16KB?**:
    *   Accommodates ~100 complex signals + 50 nodes.
    *   Small enough that `malloc(16384)` usually succeeds even after days of uptime.
*   **Risk**: If we requested 64KB, we would likely get a Generic Heap Failure due to fragmentation.

## 3. String Intering (Optimization)
In the Rules Engine, signals refer to other signals by string ID (`"rpm"`, `"boost"`).
Starnard `std::string` copies data.
Our `Signal` struct uses short-string optimization (SSO) where possible, but we encourage users to keep IDs short (4-8 chars) to stay within the localized storage of the struct and avoid heap allocations.

## 4. Stack Usage (Recursion)
Each level of the `evaluateFlow` recursion consumes stack for:
*   Function Return Address
*   Local Variables (`ParamMap`)
*   Iterators

With a default task stack of 8KB, we limit recursion to Depth 16.
`16 * 256 bytes per frame = 4KB` (Safe).
If we allowed Depth 100, we would smash the stack guarding canary.
