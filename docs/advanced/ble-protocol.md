# BLE Communication Protocol

This page describes the low-level protocol used between the W4RPBLE library and the mobile app.

## UUIDs
These IDs are public and used to discover the module services.

*   **Service**:         `0000fff0-5734-5250-5734-525000000000`
*   **RX Char** (App -> Module): `0000fff1-5734-5250-5734-525000000000` (Write)
*   **TX Char** (Module -> App): `0000fff2-5734-5250-5734-525000000000` (Notify)
*   **Status Char** (Module -> App): `0000fff3-5734-5250-5734-525000000000` (Notify)

## Command Reference

All commands are text-based ascii strings sent to the **RX Characteristic**.

### 1. `GET:PROFILE`
Requests the module's definition (capabilities, rules support, metadata).
*   **Response**: The module will stream a JSON object via `TX Char`.

### 2. `SET:RULES:<MODE>:<SIZE>:<CRC32>`
Initiates an upload of a new Ruleset.
*   `MODE`: `NVS` (save to flash) or `RAM` (temporary).
*   `SIZE`: Total size of the JSON in bytes.
*   `CRC32`: Hex string of the CRC32 checksum.
*   **Example**: `SET:RULES:NVS:1024:AABBCCDD`

### 3. `RESET:BLE`
Forces the BLE Advertising to restart. Useful if the connection is glitchy.

## Streaming Protocol (Chunked Transfer)

Because BLE packets are small (MTU is variable, typically 20-512 bytes), large payloads are "Streamed".

### Download (Module -> App)
Used for Profile and Debug logs.
1.  **Header**: Module notifies `BEGIN`.
2.  **Chunks**: Module notifies raw chunks of data (up to 180 bytes each).
3.  **Footer**: Module notifies `END:<TOTAL_BYTES>:<CRC32>`.
    *   The App **MUST** buffer all chunks until `END` is received.
    *   The App **MUST** verify the CRC32 of the reassembled buffer.

### Upload (App -> Module)
Used for sending new Rules.
1.  **Header**: App writes `SET:RULES...` (see above).
2.  **Chunks**: App writes raw chunks of JSON.
3.  **Footer**: App writes `END`.
    *   The Module buffers data and calculates CRC on the fly.
    *   On `END`, the Module verifies CRC. If valid, it parses the JSON and applies rules.
