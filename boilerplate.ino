/**
 * boilerplate.ino
 *
 * A template for W4RPBLE automotive modules.
 * This example demonstrates:
 * 1. 4-Channel Relay Control (Digital Output)
 * 2. PWM Control (Analog Output)
 * 3. Proper W4RPBLE Lifecycle (Configure -> Begin -> Register)
 */

#include <W4RPBLE.h>

W4RPBLE w4rp;

// --- Hardware Configuration ---
// Adjust pins for your specific board (e.g. ESP32-C3 SuperMini)
#define PIN_RELAY_1 0
#define PIN_RELAY_2 1
#define PIN_RELAY_3 3
#define PIN_RELAY_4 4
#define PIN_PWM_OUT 5

// PWM Config
#define PWM_FREQ 1000 // 1 kHz
#define PWM_RES 10    // 10-bit resolution (0-1023)

void setup() {
  Serial.begin(115200);

  // 1. Hardware Initialization
  // Initialize Relays (assuming Active HIGH)
  pinMode(PIN_RELAY_1, OUTPUT);
  digitalWrite(PIN_RELAY_1, LOW);
  pinMode(PIN_RELAY_2, OUTPUT);
  digitalWrite(PIN_RELAY_2, LOW);
  pinMode(PIN_RELAY_3, OUTPUT);
  digitalWrite(PIN_RELAY_3, LOW);
  pinMode(PIN_RELAY_4, OUTPUT);
  digitalWrite(PIN_RELAY_4, LOW);

  // Initialize PWM
  ledcAttach(PIN_PWM_OUT, PWM_FREQ, PWM_RES);
  ledcWrite(PIN_PWM_OUT, 0);

  // 2. Module Configuration
  // Set identity before begin()
  w4rp.setBleName("My Control Module");
  w4rp.setModuleHardware("W4RP-Relay-Box");
  w4rp.setModuleFirmware("1.0.0");
  w4rp.setCanMode(W4RPBLE::CanMode::LISTEN_ONLY); // Start SAFE

  // 3. Start W4RP Engine
  w4rp.begin();

  // 4. Register Capabilities
  // This links the "text" rules to "C++" functions
  registerRelayCapability();
  registerPwmCapability();
}

/**
 * Capability: Relay Control
 * ID: "set_relay"
 * Params:
 *  - channel: 1-4
 *  - state: "ON" or "OFF"
 */
void registerRelayCapability() {
  W4RPBLE::CapabilityMeta meta;
  meta.id = "set_relay";
  meta.label = "Control Relay";
  meta.category = "output";

  // Param: Channel Selection
  W4RPBLE::CapabilityParamMeta p_ch;
  p_ch.name = "channel";
  p_ch.type = "int";
  p_ch.min = 1;
  p_ch.max = 4;
  p_ch.description = "Relay Number (1-4)";
  meta.params.push_back(p_ch);

  // Param: State
  W4RPBLE::CapabilityParamMeta p_state;
  p_state.name = "state";
  p_state.type = "string"; // "ON" or "OFF"
  p_state.description = "Target State";
  meta.params.push_back(p_state);

  w4rp.registerCapability(meta, [](const W4RPBLE::ParamMap &params) {
    // 1. Parse Channel
    if (params.find("channel") == params.end())
      return;
    int ch = params.at("channel").toInt();

    // 2. Parse State
    bool state = false;
    if (params.find("state") != params.end()) {
      String s = params.at("state");
      if (s == "ON" || s == "TRUE" || s == "1")
        state = true;
    }

    // 3. Execute
    int pin = -1;
    switch (ch) {
    case 1:
      pin = PIN_RELAY_1;
      break;
    case 2:
      pin = PIN_RELAY_2;
      break;
    case 3:
      pin = PIN_RELAY_3;
      break;
    case 4:
      pin = PIN_RELAY_4;
      break;
    }

    if (pin != -1) {
      digitalWrite(pin, state ? HIGH : LOW);
      Serial.printf("[RELAY] Ch %d -> %s\n", ch, state ? "ON" : "OFF");
    }
  });
}

/**
 * Capability: PWM Control
 * ID: "set_pwm"
 * Params:
 *  - duty: 0-100 (%)
 */
void registerPwmCapability() {
  W4RPBLE::CapabilityMeta meta;
  meta.id = "set_pwm";
  meta.label = "Set PWM Duty";
  meta.category = "output";

  W4RPBLE::CapabilityParamMeta param;
  param.name = "duty";
  param.type = "int";
  param.min = 0;
  param.max = 100;
  meta.params.push_back(param);

  w4rp.registerCapability(meta, [](const W4RPBLE::ParamMap &params) {
    if (params.find("duty") == params.end())
      return;
    int percent = params.at("duty").toInt();

    // Clamp
    if (percent < 0)
      percent = 0;
    if (percent > 100)
      percent = 100;

    // Convert % to DAC value
    uint32_t max_val = (1 << PWM_RES) - 1;
    uint32_t duty = (percent * max_val) / 100;

    ledcWrite(PIN_PWM_OUT, duty);
    Serial.printf("[PWM] %d%% (raw=%d)\n", percent, duty);
  });
}

void loop() {
  // Keep the loop non-blocking!
  w4rp.loop();
}
