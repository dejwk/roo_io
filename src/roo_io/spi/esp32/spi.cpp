#include "roo_io/spi/esp32/spi.h"
#if defined(ESP_PLATFORM) && !defined(ARDUINO)
#include <limits.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

namespace roo_io {
Esp32SpiSlaveDevice::Esp32SpiSlaveDevice(Esp32SpiMasterBusHandle bus,
                                         int chip_select, uint32_t frequency,
                                         uint8_t mode)
    : host_(bus.host_),
      chip_select_(chip_select),
      frequency_(frequency),
      mode_(mode) {}

Esp32SpiSlaveDevice::~Esp32SpiSlaveDevice() {
  if (device_ != nullptr) spi_bus_remove_device(device_);
}

bool Esp32SpiSlaveDevice::init() {
  if (device_ != nullptr) return true;
  if (chip_select_ < 0 || frequency_ == 0 || frequency_ > INT_MAX ||
      mode_ > 3) {
    return false;
  }
  const gpio_num_t cs = static_cast<gpio_num_t>(chip_select_);
  if (gpio_set_level(cs, 1) != ESP_OK ||
      gpio_set_direction(cs, GPIO_MODE_OUTPUT) != ESP_OK) {
    return false;
  }
  spi_device_interface_config_t config = {};
  config.clock_speed_hz = static_cast<int>(frequency_);
  config.mode = mode_;
  // Manual CS lets scope destruction deassert it without an extra SPI command
  // or depending on hardware CS_KEEP_ACTIVE state after a failed transfer.
  config.spics_io_num = -1;
  config.queue_size = 1;
  spi_device_handle_t device = nullptr;
  if (spi_bus_add_device(host_, &config, &device) != ESP_OK) return false;
  device_ = device;
  return true;
}

bool Esp32SpiSlaveDevice::beginTransaction() const {
  if (device_ == nullptr || active_) return false;
  if (spi_device_acquire_bus(device_, static_cast<uint32_t>(portMAX_DELAY)) !=
      ESP_OK) {
    return false;
  }
  if (gpio_set_level(static_cast<gpio_num_t>(chip_select_), 0) != ESP_OK) {
    spi_device_release_bus(device_);
    return false;
  }
  active_ = true;
  return true;
}

bool Esp32SpiSlaveDevice::endTransaction() const {
  const bool success =
      gpio_set_level(static_cast<gpio_num_t>(chip_select_), 1) == ESP_OK;
  spi_device_release_bus(device_);
  active_ = false;
  return success;
}

bool Esp32SpiSlaveDevice::transferSelected(const uint8_t* tx, uint8_t* rx,
                                           size_t len) const {
  if (!active_ || len == 0 || len > SIZE_MAX / 8 ||
      (tx == nullptr && rx == nullptr)) {
    return false;
  }
  spi_transaction_t transaction = {};
  transaction.length = len * 8;
  transaction.tx_buffer = tx;
  transaction.rx_buffer = rx;
  return spi_device_polling_transmit(device_, &transaction) == ESP_OK;
}

bool Esp32SpiSlaveDevice::transfer(const uint8_t* tx, uint8_t* rx,
                                   size_t len) const {
  if (len == 0 || len > SIZE_MAX / 8 || (tx == nullptr && rx == nullptr)) {
    return false;
  }
  Transaction scope(*this);
  const bool success = scope.transfer(tx, rx, len);
  return scope.close() && success;
}
}  // namespace roo_io
#endif
