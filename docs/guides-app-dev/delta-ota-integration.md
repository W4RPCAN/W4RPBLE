# Delta OTA Integration

This guide explains how to implement the client-side logic for Delta Over-The-Air updates.

## The Concept

To save BLE bandwidth, we do not upload the full firmware (1MB).
We upload a **Patch** that transforms `v1.0.0` -> `v1.0.1`.
This patch is typically 5% of the size (50KB vs 1MB).

## The Prerequisites

To perform an update, your app needs access to a repository of patches.
The repository must be structured to answer: *I have v1.0.0. How do I get to v1.0.1?*

### Artifact Generation
You can generate patches using `janpatch-cli` (or any JojoDiff-compatible tool).
```bash
janpatch create old_fw.bin new_fw.bin patch_1_0_0_to_1_0_1.bin
```

## The Update Flow

### 1. Version Check
Connect to the module and perform the [Handshake](connecting-handshake.md).
Read `profile.module.fw`.
*   Example: `"fw": "1.0.0"`

### 2. Patch Selection
Check your cloud/local DB. Is there a newer version?
Is there a patch from `1.0.0` -> `1.1.0`?
*   If Yes: Download `patch_1.0_0_to_1.1.0.bin`.
*   If No (Jump too large): You might need to chain patches or perform a Full Update (slow).

### 3. Send OTA Header
Prepare the header.
*   **Size**: Size of the `.bin` patch file.
*   **CRC32**: Checksum of the **Source Firmware** (v1.0.0) that the module is *currently running*.
    *   *Correction*: The protocol actually requests the CRC of the **Patch File** in the header so the stream is validated. The module *internally* validates the Source CRC against its partition.

Command:
`OTA:BEGIN:<PATCH_SIZE>:<PATCH_CRC32>`

### 4. Stream
Stream the file using the chunking logic (180 bytes).
This will take ~10-30 seconds depending on patch size.

### 5. Completion
*   Module sends `END:<LEN>:<CRC>`.
*   App validates.
*   **Wait**: The module will SILENTLY disconnect after ~2 seconds to reboot.
*   **UI**: Show "Rebooting...".
*   **Reconnect**: Scan again, connect, and verify `profile.module.fw` is now `1.1.0`.

## Handling Failures

*   **CRC Mismatch (3001)**: The patch was corrupted in transit. Retry.
*   **Source Verification Failed**: The module rejected the update because the patch is not compatible with the installed firmware. (e.g. Trying to apply a v2->v3 patch on a v1 device).
