# Ecosystem Philosophy

Our goal is to create an open standard for automotive aftermarket modules.

## The Problem
Traditionally, aftermarket modules (valve controllers, air suspension) come with:
1.  A proprietary remote (lost easily).
2.  A proprietary app (badly maintained, disappears from App Store).
3.  Zero interoperability (Exhaust App can't talk to Suspension App).

## The W4RP Solution
We provide a **Standardized Layer** (`W4RPBLE`) that abstracts the complexity of CAN and BLE.

### 1. For Hardware Developers
You focus on your hardware (Relays, Solenoids, Sensors).
You include `W4RPBLE.h` and register your capabilities via `w4rp.registerCapability()`.
**You get:**
*   A rock-solid BLE protocol.
*   OTA Updates.
*   A ready-made App (W4RP Automotive) that works instantly with your device.

### 2. For App Developers
You can build your own dedicated app for your brand.
*   We document the full [BLE Protocol](../guides-app-dev/protocol-specification.md).
*   We provide the logic for the Rules Engine Client.
*   **Result**: You can build a "Brand X Exhaust" app that uses the same robust backend as W4RP, without being dependent on our servers.

## OTA Freedom
The OTA system is **decentralized**.
*   **Our App**: Uses our Supabase backend to fetch signed firmware for known modules.
*   **Your App**: Can fetch firmware from your own S3 bucket, GitHub Release, or local file.
*   **The Module**: Doesn't care. It just verifies the CRC32 and cryptographic signature (if enabled) of the binary stream provided by the phone.
