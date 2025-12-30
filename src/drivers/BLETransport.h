/**
 * @file BLETransport.h
 * @brief DRIVERS:BLETransport - ESP32 BLE transport
 * @version 1.0.0
 *
 * Implements Communication interface with Nordic UART-style service.
 * Service: 0000fff0-5734-5250-5734-525000000000
 *   RX:     0000fff1-... (Write)
 *   TX:     0000fff2-... (Notify)
 *   Status: 0000fff3-... (Notify)
 */
#pragma once
#include "../interfaces/Communication.h"
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

namespace W4RP {

#define W4RP_SERVICE_UUID "0000fff0-5734-5250-5734-525000000000"
#define W4RP_RX_UUID "0000fff1-5734-5250-5734-525000000000"
#define W4RP_TX_UUID "0000fff2-5734-5250-5734-525000000000"
#define W4RP_STATUS_UUID "0000fff3-5734-5250-5734-525000000000"

/**
 * @class BLETransport
 * @brief ESP32 BLE transport driver
 */
class BLETransport : public Communication,
                     public BLEServerCallbacks,
                     public BLECharacteristicCallbacks {
public:
  BLETransport();
  ~BLETransport();

  /**
   * @brief Init BLE, create service, start advertising
   * @param deviceName Advertised device name
   * @return true on success
   */
  bool begin(const char *deviceName) override;

  /**
   * @brief Check client connection
   * @return true if connected
   */
  bool isConnected() const override { return connected_; }

  /**
   * @brief Send via TX characteristic (chunked)
   * @param data Data buffer
   * @param len Data length
   */
  void send(const uint8_t *data, size_t len) override;

  /**
   * @brief Send via Status characteristic
   * @param data Data buffer
   * @param len Data length
   */
  void sendStatus(const uint8_t *data, size_t len) override;

  /**
   * @brief Set RX callback
   * @param callback Receive handler
   */
  void onReceive(TransportRxCallback callback) override {
    rxCallback_ = callback;
  }

  /**
   * @brief Set connection callback
   * @param callback Connection handler
   */
  void onConnectionChange(TransportConnCallback callback) override {
    connCallback_ = callback;
  }

  /**
   * @brief Restart advertising after disconnect
   */
  void loop() override;

  /**
   * @brief Get MTU size
   * @return 128 (safe chunk size)
   */
  size_t getMTU() const override;

  void onConnect(BLEServer *server) override;
  void onDisconnect(BLEServer *server) override;
  void onWrite(BLECharacteristic *characteristic) override;

private:
  BLEServer *server_ = nullptr;
  BLEService *service_ = nullptr;
  BLECharacteristic *rxChar_ = nullptr;
  BLECharacteristic *txChar_ = nullptr;
  BLECharacteristic *statusChar_ = nullptr;

  TransportRxCallback rxCallback_;
  TransportConnCallback connCallback_;

  bool connected_ = false;
  bool initialized_ = false;
  uint32_t lastDisconnectMs_ = 0;
  String deviceName_;

  void startAdvertising();
};

} // namespace W4RP
