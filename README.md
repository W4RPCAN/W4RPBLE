# W4RP

**Automotive CAN Bus Rule Engine for ESP32**

Read vehicle signals • Trigger custom actions • OTA updates via BLE

<p>
  <a href="#quick-start">Quick Start</a> •
  <a href="#features">Features</a> •
  <a href="#installation">Installation</a> •
  <a href="#documentation">Documentation</a> •
  <a href="#examples">Examples</a>
</p>


## Quick Start

```cpp
#include <W4RP.h>

using namespace W4RP;

TWAICanBus canBus(GPIO_NUM_21, GPIO_NUM_20);
NVSStorage storage;
BLETransport transport;
Controller w4rp(&canBus, &storage, &transport);

#define VALVE_OPEN_PIN  4  
#define VALVE_CLOSE_PIN 5  

void valveOpen() {
  digitalWrite(VALVE_CLOSE_PIN, LOW);
  digitalWrite(VALVE_OPEN_PIN, HIGH);
}

void valveClose() {
  digitalWrite(VALVE_OPEN_PIN, LOW);
  digitalWrite(VALVE_CLOSE_PIN, HIGH);
}

void valveStop() {
  digitalWrite(VALVE_OPEN_PIN, LOW);
  digitalWrite(VALVE_CLOSE_PIN, LOW);
}

CapabilityMeta exhaustMeta = {
  .id = "exhaust",
  .label = "Exhaust Valve",
  .description = "Control exhaust valve position",
  .category = "outputs",
  .params = {
    {
      .name = "state",
      .type = "int",
      .required = true,
      .min = 0,
      .max = 1,
      .description = "0=closed, 1=open"
    }
  }
};

void onExhaust(const ParamMap &params) {
  auto it = params.find("p0");
  if (it != params.end()) {
    int state = it->second.toInt();
    if (state == 1) {
      valveOpen();
    } else {
      valveClose();
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(VALVE_OPEN_PIN, OUTPUT);
  pinMode(VALVE_CLOSE_PIN, OUTPUT);
  valveClose();
  
  w4rp.setModuleInfo("EXHAUST_V1", "1.0.0", "SN-001");
  w4rp.registerCapability("exhaust", onExhaust, exhaustMeta);
  w4rp.begin();
}

void loop() { w4rp.loop(); }
```

Tested on ESP32, ESP32-C3, and ESP32-S3.

## Features

| **CAN Bus** | **Rule Engine** | **Connectivity** | **Updates** |
|-------------|-----------------|------------------|-------------|
| 500kbps vehicle CAN | Signal → Condition → Action | BLE transport | Delta OTA (10x smaller) |
| Listen-only mode | Debounce & cooldown | Nordic UART service | Full firmware OTA |
| Extended frame support | Up to 32 conditions per rule | Custom transports | Background patching |

## Installation

### Arduino IDE

1. Download this repository as ZIP
2. Sketch → Include Library → Add .ZIP Library
3. Select the W4RP-BLE folder

### PlatformIO

```ini
lib_deps = 
    https://github.com/w4rp/W4RP-BLE.git
```

### Manual

```bash
cd ~/Documents/Arduino/libraries
git clone https://github.com/w4rp/W4RP-BLE.git
```

## Architecture

```
Your Sketch
    │
    ▼
┌─────────────────────────────┐
│  Controller (W4RP.h)        │  ← Orchestrator
├─────────────────────────────┤
│  Engine   Protocol   Types  │  ← Core logic
├─────────────────────────────┤
│  CAN  Storage  Comm  OTA    │  ← Interfaces (contracts)
├─────────────────────────────┤
│  TWAI  NVS  BLE  ESP32OTA   │  ← ESP32 Drivers
└─────────────────────────────┘
```

**Dependency Injection** — You pick the drivers:

```cpp
// Use ESP32 defaults
TWAICanBus canBus(GPIO_NUM_21, GPIO_NUM_20);
Controller w4rp(&canBus, &storage, &transport);

// Or roll your own
MCP2515CanBus canBus(CS_PIN);  // SPI CAN chip
Controller w4rp(&canBus, &storage, &transport);
```

## Documentation

| Guide | Description |
|-------|-------------|
| [Quick Start](docs/getting-started/quick-start.md) | First module in 5 minutes |
| [Architecture](docs/getting-started/architecture.md) | Layer structure |
| [Capabilities](docs/getting-started/capabilities.md) | Custom action handlers |
| [Rule Engine](docs/core/rule-engine.md) | Signals, conditions, rules |
| [WBP Protocol](docs/core/wbp-protocol.md) | Binary format spec |

### API Reference

| Reference | Description |
|-----------|-------------|
| [Controller](docs/api/controller.md) | Main orchestrator |
| [Engine](docs/api/engine.md) | Rule evaluation |
| [Interfaces](docs/api/interfaces.md) | CAN, Storage, Communication, OTA |

### Drivers

| Driver | Description |
|--------|-------------|
| [CAN](docs/drivers/can.md) | TWAICanBus |
| [Storage](docs/drivers/storage.md) | NVSStorage |
| [Communication](docs/drivers/communication.md) | BLETransport |
| [OTA](docs/drivers/ota.md) | ESP32OTAService |

## Examples

| Example | Description |
|---------|-------------|
| [`examples/OTA/`](examples/OTA/) | Full + Delta firmware updates |

## Hardware

Coming soon.

## Requirements

- ESP32 / ESP32-C3 / ESP32-S3
- Arduino IDE 1.8+ or PlatformIO
- ESP32 Arduino Core 2.0+

### For Delta OTA (optional)

- [janpatch](https://github.com/janjongboom/janpatch) — Download `janpatch.h` to project folder

## Contributing

Contributions welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

MIT