/**
 * @file BLETransport.cpp
 * @brief ESP32 BLE (Bluetooth Low Energy) transport implementation
 */

#include "BLETransport.h"
#include <esp_log.h>

static const char *TAG = "BLETransport";

namespace W4RP {

BLETransport::BLETransport() {}

BLETransport::~BLETransport() {
  // BLE objects are managed by ESP-IDF, don't delete
}

bool BLETransport::begin(const char *deviceName) {
  if (initialized_)
    return true;

  deviceName_ = String(deviceName);

  // Initialize BLE
  BLEDevice::init(deviceName);
  BLEDevice::setMTU(247);

  // Create server
  server_ = BLEDevice::createServer();
  server_->setCallbacks(this);

  // Create service
  service_ = server_->createService(W4RP_SERVICE_UUID);

  // RX Characteristic (Write from client)
  rxChar_ = service_->createCharacteristic(
      W4RP_RX_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  rxChar_->setCallbacks(this);

  // TX Characteristic (Notify to client)
  txChar_ = service_->createCharacteristic(
      W4RP_TX_UUID,
      BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  txChar_->addDescriptor(new BLE2902());

  // Status Characteristic (Notify to client)
  statusChar_ = service_->createCharacteristic(
      W4RP_STATUS_UUID,
      BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  statusChar_->addDescriptor(new BLE2902());

  // Start service
  service_->start();

  // Start advertising
  startAdvertising();

  initialized_ = true;
  ESP_LOGI(TAG, "BLE initialized as '%s'", deviceName);
  return true;
}

void BLETransport::startAdvertising() {
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(W4RP_SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06); // 7.5ms
  advertising->setMaxPreferred(0x12); // 15ms
  advertising->start();
  ESP_LOGI(TAG, "Advertising started");
}

void BLETransport::onConnect(BLEServer *server) {
  connected_ = true;
  ESP_LOGI(TAG, "Client connected");

  if (connCallback_) {
    connCallback_(true);
  }
}

void BLETransport::onDisconnect(BLEServer *server) {
  connected_ = false;
  lastDisconnectMs_ = millis();
  ESP_LOGI(TAG, "Client disconnected");

  if (connCallback_) {
    connCallback_(false);
  }
}

void BLETransport::onWrite(BLECharacteristic *characteristic) {
  if (characteristic != rxChar_)
    return;

  String value = characteristic->getValue();
  if (value.length() == 0)
    return;

  if (rxCallback_) {
    rxCallback_((const uint8_t *)value.c_str(), value.length());
  }
}

void BLETransport::send(const uint8_t *data, size_t len) {
  if (!connected_ || !txChar_)
    return;

  // Chunk if needed
  size_t mtu = getMTU();
  size_t offset = 0;

  while (offset < len) {
    size_t chunkLen = (len - offset > mtu) ? mtu : (len - offset);
    txChar_->setValue((uint8_t *)(data + offset), chunkLen);
    txChar_->notify();
    offset += chunkLen;

    // Small delay between chunks to prevent overflow
    if (offset < len) {
      delay(5);
    }
  }
}

void BLETransport::sendStatus(const uint8_t *data, size_t len) {
  if (!connected_ || !statusChar_)
    return;

  statusChar_->setValue((uint8_t *)data, len);
  statusChar_->notify();
}

void BLETransport::loop() {
  if (!initialized_)
    return;

  // Restart advertising after disconnect (with delay)
  if (!connected_ && lastDisconnectMs_ > 0) {
    if (millis() - lastDisconnectMs_ > 1000) {
      startAdvertising();
      lastDisconnectMs_ = 0;
    }
  }
}

size_t BLETransport::getMTU() const {
  // Safe chunk size for notifications
  // ESP32 negotiates up to 247, but we use 128 for safety
  return 128;
}

} // namespace W4RP
