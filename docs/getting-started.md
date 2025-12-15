# Getting Started

## Installation

### Arduino IDE
1. Download the latest release `.zip` file.
2. Go to **Sketch** -> **Include Library** -> **Add .ZIP Library...**
3. Select the downloaded file.

### PlatformIO
Add the following to your `platformio.ini`:
```ini
[env:esp32-c3-devkitm-1]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
lib_deps =
    bblanchon/ArduinoJson@^6.0.0
    ; Add path to local library if not published
```

## Quick Start Code

Here is a template to get you started with your own module.

```cpp
#include <W4RPBLE.h>

W4RPBLE w4rp;

// 1. Define your action handler
void onActionTriggered(const W4RPBLE::ParamMap &params) {
    if (params.count("value")) {
        int val = params.at("value").toInt();
        Serial.printf("Action triggered with value: %d\n", val);
        // Do something real here (e.g. move text, PWM, etc)
    }
}

void setup() {
  Serial.begin(115200);

  // 2. Configure Module Identity
  w4rp.setBleName("My Custom Module");
  w4rp.setModuleFirmware("0.1.0");
  
  // 3. Configure CAN Mode (Safe for vehicles)
  w4rp.setCanMode(W4RPBLE::CanMode::LISTEN_ONLY);
  
  // 4. Initialize Core
  w4rp.begin();
  
  // 5. Register Capabilities (Actions)
  // This tells the App what commands this module supports.
  W4RPBLE::CapabilityMeta meta;
  meta.id = "custom_action";
  meta.label = "Custom Action";
  meta.category = "output";
  
  W4RPBLE::CapabilityParamMeta param;
  param.name = "value";
  param.type = "int";
  param.min = 0; 
  param.max = 100;
  meta.params.push_back(param);

  // Link the metadata to your handler function
  w4rp.registerCapability(meta, onActionTriggered);
}

void loop() {
  w4rp.loop();
}
```
