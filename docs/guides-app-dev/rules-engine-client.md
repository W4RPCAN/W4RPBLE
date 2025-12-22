# Signals, Nodes, and Flows

The W4RPBLE data model is built around three core primitives that allow you to define complex automation logic without writing C++ code.

## 1. Signals (Inputs)
A **Signal** represents a single piece of data extracted from the CAN bus.

*   **Definition**: It tells the system *how* to read a value from a raw CAN frame.
*   **Properties**:
    *   `can_id`: The 11-bit or 29-bit CAN ID.
    *   `start_bit`, `len`: Bitwise location.
    *   `factor`, `offset`: Physical scaling (`y = mx + b`).
    *   `endianness`: Motorola (Big) or Intel (Little).

> **Example**: "Engine RPM" is a 2-byte integer starting at bit 24 of Frame 0x201, multiplied by 0.25.

## 2. Nodes (Logic)
A **Node** is a single logical operation.

*   **Condition Node**: Compares a Signal against a Threshold.
    *   Operators: `>`, `<`, `==`, `WITHIN`, `OUTSIDE`.
*   **Action Node**: Triggers a [Capability](../api-reference/capabilities.md).
    *   e.g., "Set Relay to ON".
*   **Structure**: Nodes are linked together in a graph. If Node A is true, it passes execution to Node B.

## 3. Flows (Structure)
A **Flow** groups Nodes into an executable unit with timing constraints.

*   **Root Nodes**: The entry points. A Flow is considered "Active" if **ALL** its root nodes are currently True.
*   **Timing Control**:
    *   **Debounce (`hold_ms`)**: A condition must remain True for X ms before the flow fires. This filters out transient noise.
    *   **Cooldown (`cooldown_ms`)**: After firing, the flow enters a refraction period where it cannot fire again. This protects mechanical components (like solenoids) from rapid toggling.
