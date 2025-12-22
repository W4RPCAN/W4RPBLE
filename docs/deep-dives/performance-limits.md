# Deep Dive: Performance & Limits

Optimizing C++ for real-time automotive buses requires breaking typical Arduino habits.

## 1. The 1ms Deadline (CAN Throughput)

A 500kbps CAN bus runs at roughly **2000 frames per second** (theoretical max for standard IDs is higher, but 2000 is typical active load).
This means a new frame arrives every **500 microseconds**.
If your `loop()` takes **1ms** to run because of BLE processing or `delay()`, you **will lose data**.

### The Solution: Aggressive Queue Draining
The ESP32 TWAI driver has an internal RX queue (configured via `rx_queue_len`). If this fills up, hardware drops frames.
To prevent this, `W4RPBLE::loop()` executes a **Burst Drain**:

```cpp
// Pseudocode of the internal logic
void processCan() {
  int processed = 0;
  while (processed < 128 && twai_receive(&message, 0) == ESP_OK) {
      handleFrame(message); // O(1)
      processed++;
  }
}
```
*   **Why 128?**: It covers the worst-case burst size between loop iterations while yielding fast enough to keep the BLE watchdog happy.
*   **Why not infinite?**: We must return control to the FreeRTOS scheduler to handle BLE events.

## 2. Signal Processing (O(1) Complexity)

**Naive Approach (O(N))**:
```cpp
for (signal : signals) {
  if (signal.can_id == frame.id) update(signal);
}
```
With 128 signals and 2000 fps, this is `256,000` checks per second. Too slow.

**W4RP Approach (O(1))**:
We use a pre-computed Hash Map (`std::map<uint32_t, std::vector<Signal*>>`).
```cpp
auto it = signal_map.find(frame.id);
if (it != signal_map.end()) {
   // Only iterate the 1-2 signals that use this specific ID
   update(it->second);
}
```
The lookup time is constant regardless of how many rules are loaded.

## 3. BLE Throughput vs Latency

**The Bottleneck**: BLE bandwidth is low (~4KB/s safely on Android).
**The Risk**: Flooding the TX queue with `DEBUG:WATCH` packets for a signal changing 100 times/sec (like RPM).

**Congestion Control**:
1.  **Deduplication**: We only send `D:S` if the value *actually changes*.
2.  **Dirty Checking**: We check `last_debug_value` before queuing a notify.
3.  **Connection Interval**: We aggressively negotiate `min_interval=6` (7.5ms) with the central to prefer Latency over Bandwidth.

## 4. Stack Safety (Cycle Detection)

Flows allows users to link Nodes: A -> B -> C.
Users can create loops: A -> B -> A.
Infinite recursion = **Stack Overflow** = WDT Reset = Crash.

**Protection**:
We pass a `depth` counter in the recursive evaluator.
```cpp
if (depth > 16) {
    LOG_ERR("Recursion Limit Needed");
    return;
}
```
**Why 16?**: ESP32 Stack size is configurable, but 16 calls of our evaluator struct fits safely within the standard 4KB/8KB task stack.
