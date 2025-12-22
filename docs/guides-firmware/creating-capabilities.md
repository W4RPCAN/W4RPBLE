# Creating Capabilities

A **Capability** is the bridge between the Rules Engine and your actual hardware. It defines "What this module can do".

## The Anatomy of a Capability

A capability consists of two parts:
1.  **Metadata**: Describes the capability to the App (UI generation).
2.  **Handler**: The C++ code that executes the action.

## Step 1: Define Metadata

The `CapabilityMeta` struct tells the phone what UI elements to render.

```cpp
W4RPBLE::CapabilityMeta meta;
meta.id = "exhaust_flap";      // Technical ID (used in JSON rules)
meta.label = "Exhaust Valve";  // User-facing Label
meta.category = "output";      // Grouping
```

### Parameters
Capabilities take arguments. You must define them.

```cpp
W4RPBLE::CapabilityParamMeta param;
param.name = "position";
param.type = "int";           // "int", "float", "bool", "string"
param.min = 0;
param.max = 100;
meta.params.push_back(param);
```
*   **App Behavior**: If you define `min=0, max=100`, the App will render a **Slider**. If you define `type="bool"`, it renders a **Switch**.

## Step 2: Write the Handler

The handler receives a `ParamMap` (map of strings).

```cpp
void onExhaustFlap(const W4RPBLE::ParamMap &params) {
  // 1. Validate Keys
  if (params.find("position") == params.end()) return;
  
  // 2. Parse Value
  int pos = params.at("position").toInt();
  
  // 3. SAFETY CLAMP (Trust, but Verify)
  if (pos < 0) pos = 0;
  if (pos > 100) pos = 100;

  // 4. Hardware Action
  ledcWrite(PIN_MOTOR, pos);
}
```

> **Expert Tip**: Always sanitize input. Even if the App limits the slider to 100, a malicious user could manually send a rule with `position: 9999`. Your firmware must be robust.

## Step 3: Register

Link them together.

```cpp
w4rp.registerCapability(meta, onExhaustFlap);
```
