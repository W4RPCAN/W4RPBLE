# Rules Engine Architecture

The W4RPBLE Rules Engine is the brain of the module. It allows dynamic, reconfigurable automation without recompiling firmware.

## Design Philosophy

We chose a **Node-Based** architecture (similar to Node-RED) instead of a linear script. This allows for:
1.  **visual Representation**: The logic can be easily visualized as a graph.
2.  **Composability**: Complex behaviors can be built from simple atomic units.
3.  **Efficiency**: Evaluation is fast because we optimize the graph traversal.

## Core Components

### 1. Signals (Inputs)
A Signal represents a real-world value extracted from a CAN frame.
*   **Definition**: `CAN ID + Start Bit + Length + Factor + Offset`.
*   **Freshness**: Signals track `last_update_ms`. Rules can optionally check if a signal is "stale" (hasn't been received in X ms).

### 2. Nodes (Logic)
Nodes are the decision points.

#### Condition Nodes
Evaluate a Signal against a threshold.
*   **Operators**: `>`, `<`, `==`, `!=`, `WITHIN`, `OUTSIDE`.
*   **Example**: `EngineTemp > 105.0`
*   **Hold Time**: A condition can require the value to remain true for `hold_ms` before the node sets its output to TRUE. This functions as a **Debounce** or **Delay**.

### 3. Flows (Structure)
A Flow groups multiple nodes into a single executable unit.
*   **Root Nodes**: The entry points of the logic graph.
*   **Logic**: A Flow fires if **ALL** of its Root Nodes evaluate to TRUE (Internal `AND` logic).
*   **Cooldown**: Once triggered, a Flow can be forced to wait `cooldown_ms` before filtering again. this prevents rapid-fire toggling of mechanical components (like valves).

## Evaluation Cycle
The `loop()` method performs the following every cycle:

1.  **Process CAN**: Read all available frames from buffer, update Signal values.
2.  **Evaluate Signals**: Computed values (`raw * factor + offset`) are updated.
3.  **Evaluate Nodes**:
    *   Walk through all nodes.
    *   Check conditions against current Signal values.
    *   Update `hold_active` timers if conditions are met.
4.  **Evaluate Flows**:
    *   Check all Root Nodes for the flow.
    *   If all TRUE and `cooldown` expired -> **TRIGGER**.
    *   Trigger executes the linked **Capability**.

## Why this approach?
By decoupling the "What" (Signal) from the "When" (Node) and the "Then" (Flow), we allow the mobile app to completely rewrite the vehicle's behavior on the fly. You can change a "Sport Mode" threshold from 3000 RPM to 4000 RPM instantly via BLE, because it's just a number in a Node configuration.
