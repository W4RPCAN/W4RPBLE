# Connecting & Handshake

This guide details the sequence of events required to establish a valid session with a W4RPBLE module.

## 1. Discovery

The module advertises its **Service UUID** (`0000fff0...`).

### The Algorithm
1.  Start BLE Scan with Service UUID Filter.
2.  Parse `advertisementData` for the device name (e.g., "Exhaust Valve").
3.  Listen to RSSI to determine proximity.

> **Tip**: Android devices cache names aggressively. Always use the name from the Advertisement Payload, not the cached GATT name.

## 2. Connection

1.  **Connect**: Initiate GATT connection.
2.  **Discover Services**: Request the primary service.
3.  **Discover Characteristics**: Find RX, TX, and Status.
4.  **Subscribe**: Enable Notifications (CCCD) on **TX** and **Status**.
    *   *Critical*: If you don't subscribe, you will never receive data.

## 3. The Handshake (Get Profile)

Once connected, the app knows nothing about the module (What is it? What capabilities does it have?).
You must fetch the **Profile**.

**Request**:
Write `GET:PROFILE` to **RX**.

**Response (Stream)**:
The module will stream a large JSON object via **TX**.
See [Protocol Spec](protocol-specification.md) for chunk handling.

### Profile Schema
```json
{
  "module": {
    "id": "W4RP-A1B2C3",
    "hw": "esp32c3-mini-1",
    "fw": "0.5.0",
    "device_name": "Exhaust Valve"
  },
  "capabilities": {
    "exhaust_flap": {
      "label": "Exhaust Flap",
      "category": "output",
      "params": [
        { "name": "amount", "type": "int", "min": 0, "max": 100 }
      ]
    }
  },
  "runtime": { "mode": "nvs" }
}
```

### 4. UI Generation
Using the `capabilities` object, your app should dynamically generate controls.
*   `"type": "int", "min": 0, "max": 100` -> **Slider**.
*   `"type": "bool"` -> **Toggle Switch**.
*   `"category": "output"` -> **Control Tab**.

This allows your app to control *future* modules (e.g., Air Suspension) without needing an app update, as the UI is defined by the firmware.
