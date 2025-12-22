/**
 * @file W4RPBLE.h
 * @brief W4RPBLE - BLE-to-CAN rules engine for ESP32
 * @version 0.5.0
 *
 * A library for creating BLE-connected automotive modules that can read
 * CAN bus signals and execute rule-based flows. Supports ESP32-C3 and ESP32-S3.
 *
 * @note Requires ArduinoJson >= 6.0.0
 */

#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include <map>
#include <vector>

class BLECharacteristic;
class BLEServer;

#include <driver/twai.h>

/**
 * @class W4RPBLE
 * @brief Main library class for BLE-CAN integration with rule-based automation
 *
 * This class provides:
 * - BLE connectivity with chunked data transfer
 * - CAN bus reading via ESP32 TWAI driver
 * - Rule-based flow evaluation engine
 * - NVS persistence for rules
 * - Capability registration for custom actions
 *
 * @example
 * @code
 * W4RPBLE w4rp;
 *
 * void setup() {
 *   w4rp.setBleName("My Module");
 *   w4rp.setCanMode(W4RPBLE::CanMode::LISTEN_ONLY);
 *   w4rp.begin();
 *
 *   w4rp.registerCapability("my_action", [](const W4RPBLE::ParamMap &params) {
 *     // Handle action
 *   });
 * }
 *
 * void loop() {
 *   w4rp.loop();
 * }
 * @endcode
 */
class W4RPBLE {
public:
  /// @brief Map of parameter key-value pairs for capability handlers
  using ParamMap = std::map<String, String>;

  /// @brief Callback function type for capability handlers
  using CapabilityHandler = std::function<void(const ParamMap &)>;

  /**
   * @brief Write a value to NVS (Non-Volatile Storage)
   * @param key The storage key
   * @param value The string value to store
   * @return true if successful
   */
  bool nvsWrite(const char *key, const String &value);

  /**
   * @brief Read a value from NVS
   * @param key The storage key
   * @return The stored value, or empty string if not found
   */
  String nvsRead(const char *key);

  /// @name BLE Callbacks (internal use)
  /// @{
  void onBleConnect(BLEServer *server);
  void onBleDisconnect(BLEServer *server);
  void onBleWrite(BLECharacteristic *characteristic);
  /// @}

  /**
   * @brief Apply a ruleset from a parsed JSON document
   * @param doc The ArduinoJson document containing signals, nodes, and flows
   * @return true if ruleset was applied successfully
   */
  bool applyRuleset(JsonDocument &doc);

  /**
   * @brief Metadata for a capability parameter
   */
  struct CapabilityParamMeta {
    String name; ///< Parameter name/key
    String type; ///< Type: "int", "float", "string", "bool"
    bool required = true;
    int min = 0;        ///< Minimum value (for numeric types)
    int max = 0;        ///< Maximum value (for numeric types)
    String description; ///< Human-readable description
  };

  /**
   * @brief Metadata for a capability (action that can be triggered by rules)
   */
  struct CapabilityMeta {
    String id;          ///< Unique identifier (e.g., "exhaust_flap")
    String label;       ///< Human-readable label
    String description; ///< Detailed description
    String category;    ///< Category: "output", "debug", etc.
    std::vector<CapabilityParamMeta> params; ///< List of parameters
  };

  /// @brief Constructor
  W4RPBLE();

  /**
   * @brief Initialize the library (BLE, CAN, NVS)
   * @note Call all set* methods before this
   */
  void begin();

  /**
   * @brief Main loop - call this in your loop() function
   *
   * Processes CAN messages, evaluates rules, and handles BLE communication.
   */
  void loop();

  /// @name Module Identification
  /// @{

  /**
   * @brief Get the module ID (e.g., "W4RP-ABCDEF")
   * @return Null-terminated string
   */
  const char *getModuleId() const;

  /**
   * @brief Get the firmware version
   * @return Null-terminated string
   */
  const char *getFwVersion() const;
  /// @}

  /// @name Configuration (call before begin())
  /// @{

  /**
   * @brief Set the hardware model identifier
   * @param hw Hardware model (e.g., "esp32c3-mini-1")
   */
  void setModuleHardware(const String &hw);

  /**
   * @brief Set the firmware version string
   * @param fw Firmware version (e.g., "0.5.0-exhaust")
   */
  void setModuleFirmware(const String &fw);

  /**
   * @brief Set the module serial number
   * @param serial Serial/VIN (e.g., "AMAROK-01")
   */
  void setModuleSerial(const String &serial);

  /**
   * @brief Override the auto-generated module ID
   * @param id Custom module ID (e.g., "W4RP-CUSTOM")
   */
  void setModuleIdOverride(const String &id);

  /**
   * @brief Set the BLE advertising name
   * @param name Name visible in BLE scans (e.g., "Exhaust Valve")
   */
  void setBleName(const String &name);

  /**
   * @brief Override default GPIO pins
   * @param can_tx CAN TX pin (default: GPIO21 on C3)
   * @param can_rx CAN RX pin (default: GPIO20 on C3)
   * @param led Status LED pin (default: GPIO8 on C3)
   */
  void setPins(int8_t can_tx, int8_t can_rx, int8_t led);

  /**
   * @brief Override CAN bus timing configuration
   * @param config TWAI timing config (default: 500kbps)
   */
  void setCanTiming(const twai_timing_config_t &config);

  /**
   * @brief CAN bus operating modes
   */
  enum class CanMode {
    NORMAL,      ///< Standard TX/RX mode with ACK
    LISTEN_ONLY, ///< Receive only, no ACK (safe for vehicle buses)
    NO_ACK       ///< TX/RX without waiting for ACK
  };

  /**
   * @brief Set the CAN bus operating mode
   * @param mode The desired mode (default: NORMAL)
   * @warning Must be called before begin()
   */
  void setCanMode(CanMode mode);
  /// @}

  /// @name Capability Registration
  /// @{

  /**
   * @brief Register a simple capability handler
   * @param id Unique capability ID
   * @param handler Callback function
   */
  void registerCapability(const String &id, CapabilityHandler handler);

  /**
   * @brief Register a capability with full metadata
   * @param meta Capability metadata (for module profile)
   * @param handler Callback function
   */
  void registerCapability(const CapabilityMeta &meta,
                          CapabilityHandler handler);
  /// @}

  struct ImplState; // Forward declaration
  ImplState *impl_; // Pointer to implementation

private:
  void deriveModuleId();
  void initBle();
  void initCan();
  void loadRulesFromNvs();
  void processCan();
  void evaluateFlowsInternal();
  void sendStatusIfNeeded();
  void sendModuleProfile();
  void sendStatusUpdate();
  void blinkLed(uint8_t times, uint16_t ms);

  void sendDebugUpdates();

  void pauseOperations();
  void resumeOperations();

  static void otaWorkerTaskStatic(void *pvParameters);
  void otaWorkerTask();
};