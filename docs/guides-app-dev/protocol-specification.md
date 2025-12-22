# Protocol Specification

This document is the definitive guide for building a client application (iOS/Android/Web) that interacts with W4RPBLE modules.

## 1. Bluetooth Low Energy Service

### UUID Configuration
These UUIDs are the default for W4RP v1. Applications should allow these to be configurable to support white-labeling.

| Characteristic | UUID | Properties | Function |
|---|---|---|---|
| **Service** | `0000fff0-5734-5250-5734-525000000000` | - | Primary Service |
| **RX (Write)** | `0000fff1-5734-5250-5734-525000000000` | `WRITE`, `WRITE_WITHOUT_RESPONSE` | Phone -> Module (Commands) |
| **TX (Notify)** | `0000fff2-5734-5250-5734-525000000000` | `NOTIFY` | Module -> Phone (Streams) |
| **Status** | `0000fff3-5734-5250-5734-525000000000` | `NOTIFY` | Module -> Phone (Heartbeat) |

## 2. Command Structure (RX)

All commands are UTF-8 strings.
Large payloads (Rules, OTA) use a **Header + Chunk** streaming model.

### 2.1 Get Profile
*   **Command**: `GET:PROFILE`
*   **Response**: Module streams the Profile JSON via TX.

### 2.2 Upload Rules
Used to update the automation logic.
*   **Header**: `SET:RULES:<MODE>:<SIZE>:<CRC32>`
    *   `MODE`: `NVS` (Persistent) or `RAM` (Temporary).
    *   `SIZE`: Total bytes of JSON.
    *   `CRC32`: IEEE CRC32 of JSON.
*   **Body**: Stream JSON in chunks (max 180 bytes).
*   **Footer**: `END`
*   **Wait**: ~3-5ms between chunks to allow ESP32 processing.

### 2.3 Debug Mode
*   **Start**: `DEBUG:WATCH:<SIZE>:<CRC32>` (Followed by JSON stream of Signals to watch).
*   **Stop**: `DEBUG:STOP`

### 2.4 OTA Update
*   **Header**: `OTA:BEGIN:<SIZE>:<CRC32>`
*   **Body**: Stream Binary Patch (Janpatch format).
*   **Footer**: `END`

## 3. Streaming Logic (TX)

Because the ESP32 cannot handle large MTUs reliably across all Android phones, we chunk data at **180 bytes**.

**Download Flow (Module -> Phone)**:
1.  **Phone**: Sends `GET:PROFILE`.
2.  **Module**: Sends `BEGIN` (Legacy: `BEGIN:LEN:CRC`).
3.  **Module**: Stream chunks of JSON.
4.  **Module**: Sends `END:<LEN>:<CRC32>`.
5.  **Phone**: Concatenates chunks, validates CRC, parses JSON.

## 4. Error Codes

When building your app, utilize these standard error codes for user feedback.

| Code | Name | Description |
|---|---|---|
| `1000` | ConnectionFailed | Generic connection error. |
| `1002` | NotConnected | Attempted operation without connection. |
| `1004` | DeviceNotFound | Scan timeout. |
| `1007` | BluetoothOff | System Bluetooth is disabled. |
| `1009` | UuidNotConfigured | App failed to load UUID config. |
| `2003` | WriteFailed | GATT Write failed (Module disconnected?). |
| `3001` | CrcMismatch | Rules/Profile/OTA integrity check failed. |
| `3002` | LengthMismatch | Streamed bytes != Header size. |
| `4000` | ProfileTimeout | Module didn't reply to `GET:PROFILE`. |
| `4001` | StreamTimeout | Stream hung mid-transfer. |

## 5. Best Practices

### Connection Parameters
*   **Android**: Request `CONNECTION_PRIORITY_HIGH` (Interval ~11.25ms).
*   **iOS**: System manages it, but standard is ~30ms.
*   **Impact**: Lower interval = Faster OTA, Higher Power.

### MTU
*   Negotiate MTU to **247** (Standard max) where possible.
*   Application payload should cap at **180** to leave room for L2CAP/ATT headers safe margin.
