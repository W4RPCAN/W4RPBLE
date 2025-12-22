# Deep Dive: OTA Internals

Our OTA system is an "Open Loop" Delta Update mechanism designed for unreliable, low-bandwidth BLE links.

## 1. The Delta Algorithm (Janpatch)

We use the **Janpatch** library (a C implementation of JojoDiff).
*   **Concept**: Instead of sending the full 1MB binary, we send a ~50KB "Recipe" (Patch) that turns Version X into Version Y.
*   **Math**: `New_FW = Apply(Old_FW, Patch)`
*   **The Constraint**: To build the new file, the CPU must read the *Old Firmware* from flash, read the *Patch* from BLE, and write the *New Firmware* to flash simultaneously.

## 2. Partition Architecture

The ESP32 uses a specific flash layout (`partitions.csv`) to support this:

| Name | Type | SubType | Address | Size | Function |
|---|---|---|---|---|---|
| `nvs` | data | nvs | 0x9000 | 16KB | Rules storage |
| `otadata` | data | ota | 0xd000 | 8KB | Boot flags (selects active app) |
| `app0` | app | ota_0 | 0x10000 | 1MB | Slot A |
| `app1` | app | ota_1 | 0x110000 | 1MB | Slot B |

**The Ping-Pong**:
If running from `app0`, the updater writes to `app1`.
If running from `app1`, the updater writes to `app0`.

## 3. The Memory Bridge (Ring Buffer)

**The Problem**:
*   Flash Writes are blocking and slow.
*   BLE Packets arrive asynchronously.
*   Janpatch needs a linear data stream.
*   We have very little RAM (cannot buffer 50KB patch).

**The Solution**: A 8KB Ring Buffer (`rx_ring_buffer`).
1.  **BLE Task (Producer)**: Receives packets. Pushes to Head. If buffer full, it drops (Client must handle flow control/retry, though BLE is generally reliable with ACK).
2.  **OTA Task (Consumer)**: Janpatch asks for "Next 128 bytes". We pop from Tail. If buffer empty, we yield and wait.

## 4. Safety Mechanisms

### Source Verification (The "Brick" Preventer)
A patch file is mathematically generated *from* a specific source binary. If you apply a patch intended for v1.0 onto v1.1, the result is garbage (Brick).
*   **Mechanism**: The `OTA:BEGIN` command includes the CRC32 of the *Source Firmware*.
*   **Check**: The device calculates the CRC of its *current* running partition. If it doesn't match the header, it rejects the update immediately.

### Cryptographic Signatures (Optional)
Protocol supports appending an Ed25519 signature to the `OTA:BEGIN` header. The device can verify this against a public key burned in factory NVS to prevent unauthorized firmware.

## 5. Why not WiFi?
Using WiFi for OTA in a car is fragile:
1.  Phone must switch to AP mode (disconnects internet).
2.  Or Module uses Station mode (requires router).
3.  High power consumption during download.

BLE is slower but seamless. The user can watch TikTok while the car updates in the background.
