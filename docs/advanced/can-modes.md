# CAN Bus Modes & Filtering

The W4RPBLE library supports different CAN operating modes to ensure compatibility and safety when connected to vehicle networks.

## Available Modes

| Mode | Enum Value | Description | Use Case |
|------|------------|-------------|----------|
| **Normal** | `W4RPBLE::CanMode::NORMAL` | Standard TX/RX. The controller will acknowledge (ACK) frames it receives. | Lab bench, or when you are the main controller. |
| **Listen Only** | `W4RPBLE::CanMode::LISTEN_ONLY` | Receives frames but **does not** ACK. The TX pin is disabled. | **Connecting to a real vehicle.** Prevents bus errors if baud rate mismatches. |
| **No ACK** | `W4RPBLE::CanMode::NO_ACK` | Sends and receives but does not require ACKs. | Specific testing scenarios (Self-Test mode). |

> ⚠️ **Safety Warning:** When connecting to a vehicle OBD2 port or internal CAN wiring, ALWAYS use `LISTEN_ONLY` first. Sending data or ACKs on a bus with incorrect baud rate can trigger dashboard error lights.

## CAN Filtering Strategy

### Why Software Filtering?
You might notice W4RPBLE does not expose the hardware acceptance filters (`twai_filter_config_t`). This is a deliberate design choice.

**The Problem**:
ESP32's TWAI controller has a single hardware filter (mask/code). It works well for accepting a *range* of IDs (e.g., `0x100` to `0x1FF`), but it is very poor at accepting **disjoint groups** (e.g., "I need `0x123` AND `0x7E0`").
To accept `0x123` and `0x7E0` in hardware, you often have to open the mask so wide that you accept almost everything anyway.

**Our Solution**:
1.  **Promiscuous Mode**: We configure the hardware to accept **ALL** standard frames.
2.  **Software Dispatch**: In `processCan()`, we perform a highly optimized lookup (O(1) or O(log n)) to check if the received ID matches any defined Signal.
3.  **Efficiency**: Since we typically care about < 50 signals, the SW overhead is negligible compared to the complexity of managing hardware masks for dynamic rules.

This approach gives the Rules Engine maximum flexibility. The user doesn't need to understand binary masks; they just say "I want Signal 0x123", and the library handles it.
