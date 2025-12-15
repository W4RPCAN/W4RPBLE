# Capabilities

Capabilities are the core of the W4RPBLE automation system. They are the "actions" that your hardware receives from the W4RPBLE app or the internal Rules Engine.

## How it Works

1.  **Registration**: You define a Capability in your firmware (ID, Label, Parameters).
2.  **Discovery**: On connection, the App downloads this profile.
3.  **UI Generation**: The App automatically builds a UI for your capability (e.g., a slider for an `int`).
4.  **Triggering**:
    *   **Manual**: User interacts with the App UI.
    *   **Automated**: The Rules Engine (Flow) triggers the capability when conditions are met (e.g., `RPM > 3000`).
5.  **Execution**: The library calls your registered `handler` function with the provided parameters.

## `registerCapability`

This method links your **Metadata** (description) to your **Handler** (code).

### 1. Define Metadata
This tells the W4RP system *what* your module can do.

```cpp
W4RPBLE::CapabilityMeta meta;
meta.id = "servo_control";          // Unique ID used in JSON rules
meta.label = "Servo Position";      // Display Name in App
meta.category = "output";           // 'output', 'debug', etc.

// Define parameters (inputs for your action)
W4RPBLE::CapabilityParamMeta param;
param.name = "angle";
param.type = "int";
param.min = 0;
param.max = 180;
meta.params.push_back(param);
```

### 2. Define Handler
This function is called whenever the action is triggered.

```cpp
void onServoControl(const W4RPBLE::ParamMap &params) {
    // 1. Extract parameter
    if (params.count("angle") == 0) return;
    
    int angle = params.at("angle").toInt();
    
    // 2. Execute Hardware Action
    myServo.write(angle);
    
    LOG_BLE("Servo moved to %d", angle);
}
```

### 3. Register
Link them together in `setup()`.

```cpp
w4rp.registerCapability(meta, onServoControl);
```

## Types

### `ParamMap`
A standard map containing the parameters sent by the rule engine.
```cpp
using ParamMap = std::map<String, String>;
```

### `CapabilityHandler`
The function signature for your callbacks.
```cpp
using CapabilityHandler = std::function<void(const ParamMap &)>;
```
