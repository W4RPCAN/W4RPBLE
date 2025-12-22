# Component Lifecycle

Understanding the initialization and execution lifecycle is crucial for robust integrations.

## 1. Initialization (`begin`)

The `w4rp.begin()` sequence initializes subsystems in this specific order:

1.  **GPIO Setup**: Configures LED pin.
2.  **Identity Derivation**: Generates `module_id` from MAC address (unless overridden).
3.  **Boot Count**: Increments and saves boot count to NVS (Safety Metric).
4.  **BLE Init**: Starts GATT Server and Advertising.
5.  **CAN Init**: Installs TWAI driver with configured timings and modes.
6.  **NVS Init**: Opens the `w4rp` namespace partitions.
7.  **Rules Load**: Attempts to load the active ruleset. If loading fails (CRC mismatch), falls back to the backup slot.

## 2. The Loop (`loop`)

The `loop()` function is designed to be non-blocking. It should be called as frequently as possible.

### Step A: CAN Processing (Burst Mode)
*   **Action**: Drains up to **128 frames** from the TWAI hardware buffer.
*   **Logic**: Loops through `processCan()` which performs O(1) matching against Signal definitions.
*   **Priority**: High. Bus overruns are avoided by aggressive draining.

### Step B: Rules Evaluation
*   **Action**: `evaluateFlowsInternal()`
*   **Logic**: Checks all Flows. If a Flow's debounce timer is satisfied, executes the Node Graph.

### Step C: Transmissions
*   **Debug**: If `DEBUG:START` is active, sends `D:S` (Signal) and `D:N` (Node) updates via BLE Notify.
*   **Status**: Periodically (5s) sends the Module Profile heartbeat.

### Step D: OTA Safety
*   **Logic**: If an OTA Update is active (`ota_in_progress`), the loop **skips** Step A and B to give full CPU priority to the `otaWorkerTask` and BLE stack. This prevents flash write timeouts.
