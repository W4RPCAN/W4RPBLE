# Safety Mechanisms

Automotive environments are unforgiving. A software crash at 100km/h is unacceptable. W4RPBLE implements several layers of protection.

## 1. Connection Loss = Autonomous Mode

The most important safety feature is: **The Module does not need the Phone.**
*   **Scenario**: User is driving, phone runs out of battery.
*   **Behavior**: The ESP32 continues to read CAN and execute rules.
    *   If a rule says "Open Valve > 4000 RPM", it will still open.
*   **Design Rule**: Never design a system that relies on the phone for critical safety logic.

## 2. Fail-Safe Defaults

When the module boots (or reboots after a crash), it initializes all hardware.
**You must ensure this initial state is safe.**

*   **Exhaust**: Open (Loud but safe).
*   **Suspension**: Normal Ride Height (Don't slam to ground).
*   **Lights**: Off.

```cpp
void setup() {
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, LOW); // Fail-Safe: Open
  // ...
  w4rp.begin();
}
```

## 3. Listen-Only Integration

When users install your module, they might first just tap into the CAN wires.
*   **Risk**: If they tap the wrong wires (e.g., 250kbps chassis bus instead of 500kbps powertrain), a controller sending frames will crash the bus.
*   **Mitigation**: `W4RPBLE::CanMode::LISTEN_ONLY`.
    *   The TX pin is physically disabled. Note: The transceiver cannot pull the bus dominant.
    *   This is the "Zero Harm" mode.

## 4. Recursion Watchdog

If a user uploads a rule that creates an infinite loop (`Flow A -> Flow B -> Flow A`), the library's **Recursion Monitor** catches it at Depth 16.
*   **Action**: Aborts the flow.
*   **Log**: "Recursion Limit Exceeded".
*   **Result**: Prevents the Stack Overflow that would otherwise Reset the CPU.

## 5. Twin-Slot Persistence

What if power is cut *exactly* while saving new rules?
*   **Risk**: Corrupted JSON.
*   **Protection**: The previous valid ruleset is still in the other NVS slot.
*   **Result**: On next boot, CRC fails for Slot A, so it loads Slot B. The car still works.
