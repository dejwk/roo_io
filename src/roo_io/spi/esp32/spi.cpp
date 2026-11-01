#include "roo_io/spi/esp32/spi.h"
#if defined(ESP_PLATFORM) && !defined(ARDUINO)
#include <limits.h>
#include <stdint.h>

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
  spi_device_interface_config_t config = {};
  config.clock_speed_hz = static_cast<int>(frequency_);
  config.mode = mode_;
  config.spics_io_num = chip_select_;
  config.queue_size = 1;
  spi_device_handle_t device = nullptr;
  if (spi_bus_add_device(host_, &config, &device) != ESP_OK) return false;
  device_ = device;
  return true;
}

bool Esp32SpiSlaveDevice::transfer(const uint8_t* tx, uint8_t* rx,
                                   size_t len) const {
  if (device_ == nullptr || len == 0 || len > SIZE_MAX / 8 ||
      (tx == nullptr && rx == nullptr)) {
    return false;
  }
  spi_transaction_t transaction = {};
  transaction.length = len * 8;
  transaction.tx_buffer = tx;
  transaction.rx_buffer = rx;
  return spi_device_polling_transmit(device_, &transaction) == ESP_OK;
}
}  // namespace roo_io
#endif
