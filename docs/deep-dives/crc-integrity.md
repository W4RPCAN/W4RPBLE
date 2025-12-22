# Deep Dive: Integrity & CRC32

In an automotive environment, electromagnetic interference (EMI) from ignition coils and alternators can induce noise in data lines. We take integrity seriously.

## Hardware vs Software CRC

The ESP32 family includes a hardware accelerator for Cyclic Redundancy Checks.

*   **Software CRC**: Consumes CPU cycles. For a 1MB file, this is significant.
*   **Hardware CRC**: The `esp_crc` module computes checksums in parallel with CPU operations.
    *   **Algorithm**: IEEE-802.3 (Standard Ethernet CRC32).
    *   **Polynomial**: `0x04C11DB7`.

We use the hardware engine for:
1.  **OTA Stream Verification**: Validating the 50KB-500KB patch file on the fly.
2.  **Streaming Rules**: Checking the ruleset integrity before committing to NVS.

## Implementation Details

### The Inversion Quirk
The ESP32 HW CRC implementation (`esp_crc32_le`) often causes confusion because of how it handles endianness and bit inversion.
*   **Init Value**: We must pass `0` (not `0xFFFFFFFF`) because the HW module handles the initial inversion.
*   **Input**: Little-endian processing.

```cpp
static uint32_t crc32_ieee(const uint8_t *data, size_t len) {
  // Use 0 init because esp_crc32_le handles the input/output inversion
  return esp_crc32_le(0, data, len);
}
```

## Layers of Defense

### 1. Transport Layer (Low)
BLE itself has a CRC check at the L2CAP layer. If a packet is corrupted over the air, the radio hardware drops it before our software even sees it.

### 2. Stream Layer (Middle)
We implement a secondary CRC32 check on **Aggregated Data**.
Why? Because BLE packets might arrive out of order (though rare in Write without Response) or the phone might send garbage logic.
*   **Header**: `SET:RULES:NVS:1024:<CRC32>`
*   **Process**: We compute usage CRC as chunks arrive.
*   **End**: We compare `Calculated == Header`. If mismatch, we discard.

### 3. Storage Layer (High)
We store the CRC32 of the ruleset *alongside* the JSON in NVS.
*   **On Boot**: We read the JSON string string, compute CRC, and compare with stored CRC.
*   **Benefit**: Tells us instantly if Flash Corruption occurred without needing to parse multiple kilobytes of JSON.
