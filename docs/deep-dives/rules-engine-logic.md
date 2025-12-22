# Rules Engine Logic

The heart of W4RPBLE is a graph-traversal engine that evaluates automation logic in real-time.

## Design Philosophy

### Why JSON?
We define rules in JSON because:
1.  **Portability**: JSON is native to the Web/App layer. Your phone generates the logic, the ESP32 consumes it.
2.  **Safety**: We don't execute script code (Lua/Python). We execute a *static data structure*. This prevents runtime syntax errors crashing the vehicle's controller.

### Why DFS (Depth-First Search)?
We use DFS because automation flows are typically "Chain Reactions".
*   *If RPM > 3000 -> Open Valve -> Flash LED.*
DFS guarantees that if the condition is met, the actions happen in that exact order within the same millisecond tick.

## The Evaluation Cycle

The evaluation occurs in `W4RPBLE::loop()` effectively on every CPU cycle where data is available.

1.  **Ingestion**: `processCan()` updates the raw values of all Signals.
2.  **Evaluation**: `evaluateFlowsInternal()` iterates through all defined Flows.
3.  **Traversal**: For each Flow, we recursively traverse its Node Graph.

## Recursion & Safety

One of the biggest risks in user-defined logic is **Infinite Loops**.
*   User Rule: `If A then B. If B then A.`
*   Result: Stack Overflow.

**The Guard**:
We implemented a hard **Depth Counter**.
```cpp
bool traverse_flow_graph(node, depth) {
    if (depth > 16) return error("Recursion Limit");
    // ...
}
```
**Why 16?**: It allows for complex logic chains but guarantees we never exceed the 8KB Stack size of the FreeRTOS task.

## Debounce (Hold Time)

Real-world sensors are noisy. A throttle pedal might flicker `99% -> 100% -> 99%`.
To prevent the valve from "chattering" (opening/closing rapidly), we implemented **Hold Time** logic at the Node level.

### Logic
1.  **Rising Edge**: First time `Value > Threshold`, record `start_time = millis()`.
2.  **Holding**: We do NOT trigger the action yet. We wait.
3.  **Trigger**: Only when `millis() - start_time >= hold_ms` do we output TRUE.

This filters out transient noise spikes typical in automotive electrical systems.
