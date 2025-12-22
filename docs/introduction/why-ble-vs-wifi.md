# Why BLE vs WiFi?

One of the most common questions is: *"Why use Bluetooth Low Energy instead of WiFi for an automotive module?"*

The decision was not made lightly. We chose BLE because it aligns perfectly with the constraints and requirements of the automotive environment.

## 1. The "Zero Dependency" Requirement
A vehicle module must work in a desert, in a tunnel, or on a track.
*   **WiFi (Client Mode)**: Requires a router. A car does not reliably have a router.
*   **WiFi (AP Mode)**: Requires the phone to disconnect from 4G/5G data to connect to the module. This kills music streaming, maps, and cloud sync.
*   **BLE**: Works parallel to 4G/5G. It requires no infrastructure. The defined range (approx 10m) is perfect for a vehicle.

## 2. Multi-Module Concurrency
A robust vehicle system might have:
1.  **Exhaust Controller** (Rear)
2.  **Air Suspension** (Trunk)
3.  **Underglow** (Chassis)

*   **WiFi**: A phone can only connect to **one** Access Point at a time. To control 3 modules, you would need to switch networks constantly.
*   **BLE**: A modern smartphone can maintain connections to **5-8 peripherals simultaneously**. The W4RP App can aggregate data from the Exhaust, Suspension, and Lights into a single dashboard.

## 3. Latency & Protocol Overhead
*   **WiFi (HTTP/TCP)**: Heavy handshake. Opening a socket, sending headers, waiting for ACK. Good for Netflix, bad for a tachometer.
*   **BLE (GATT Notify)**: Once connected, the channel is open. Sending an RPM update is a simple 20-byte packet pushed immediately into the radio slot.
    *   **Result**: We achieve 20-50ms latency for gauge updates, which feels "instant" to the human eye.

## 4. Security via Proximity
WiFi signals can be amplified to attack from blocks away.
BLE requires physical proximity (~10-20 meters). This physical constraint adds a layer of security—an attacker must be effectively standing next to the car to attempt a connection.

## Summary
We use BLE because it treats the phone as a **User Interface Extension**, not a Network Client. The phone becomes the screen for the module, without interrupting the user's digital life (internet, calls, maps).
