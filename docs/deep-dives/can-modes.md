# CAN Filtering & Modes

## Operating Modes

The library supports three operating modes, selectable via `setCanMode()`.

| Mode | Enum | Use Case |
|---|---|---|
| **Normal** | `NORMAL` | **Bench/Controller**. Sends ACKs. If baud rate is wrong, it destroys traffic. |
| **Listen Only** | `LISTEN_ONLY` | **Vehicle Sniffing**. No ACKs. TX pin disabled. Safe for connecting to live cars. |
| **No ACK** | `NO_ACK` | **Self-Test**. Sends frames but doesn't wait for ACK. |

> [!CAUTION]
> Always use `LISTEN_ONLY` when first connecting to a vehicle to verify baud rate without triggering dashboard errors.

## Filtering Strategy

W4RPBLE uses **Promiscuous Mode + Software Filtering**.

### Why not Hardware Filters?
The ESP32 TWAI controller has only **one** hardware filter (Acceptance Mask/Code).
*   Hardware filters are good for ranges (`0x100` to `0x1FF`).
*   They are **bad** for specific, disjoint IDs (`0x100` AND `0x7E8`).
*   To support dynamic rulesets where users can pick *any* ID, we would have to calculate complex bitmasks on the fly, often resulting in "Open All" anyway.

### The "O(1) Map" Solution
1.  **Hardware**: Configured to `TWAI_FILTER_CONFIG_ACCEPT_ALL`.
2.  **Software**: We build a `std::map<uint32_t, std::vector<Signal*>>` during `applyRuleset`.
3.  **Process**:
    *   Read Frame ID.
    *   Look up in Map.
    *   If found -> Extract Signals.
    *   If not found -> Discard immediately.

This allows the user to listen to *any* combination of IDs without understanding binary masks, with negligible performance penalty thanks to the optimized lookup.
