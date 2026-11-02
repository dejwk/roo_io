#include "roo_io/spi/pico/spi.h"
#if defined(PICO_RP2040) && PICO_RP2040
#include <hardware/gpio.h>

#include "roo_threads/mutex.h"
namespace roo_io {
namespace {
roo::mutex locks[2];
}
PicoSpiSlaveDevice::PicoSpiSlaveDevice(const PicoSpiSlaveDevice& other)
    : PicoSpiSlaveDevice(PicoSpiMasterBusHandle(other.bus_), other.chip_select_,
                         other.frequency_, other.mode_) {
  initialized_ = other.initialized_;
}
bool PicoSpiSlaveDevice::init() {
  if ((bus_ != spi0 && bus_ != spi1) || chip_select_ < 0 ||
      chip_select_ >= 30 || frequency_ == 0 || mode_ > 3)
    return false;
  gpio_init(chip_select_);
  gpio_put(chip_select_, true);
  gpio_set_dir(chip_select_, GPIO_OUT);
  initialized_ = true;
  return true;
}
bool PicoSpiSlaveDevice::beginTransaction() const {
  if (!initialized_ || active_) return false;
  locks[bus_ == spi0 ? 0 : 1].lock();
  spi_set_baudrate(bus_, frequency_);
  spi_set_format(bus_, 8, static_cast<spi_cpol_t>(mode_ >> 1),
                 static_cast<spi_cpha_t>(mode_ & 1), SPI_MSB_FIRST);
  gpio_put(chip_select_, false);
  active_ = true;
  return true;
}
bool PicoSpiSlaveDevice::endTransaction() const {
  gpio_put(chip_select_, true);
  active_ = false;
  locks[bus_ == spi0 ? 0 : 1].unlock();
  return true;
}
bool PicoSpiSlaveDevice::transferSelected(const uint8_t* tx, uint8_t* rx,
                                          size_t len) const {
  if (!active_ || len == 0 || (tx == nullptr && rx == nullptr)) return false;
  // A byte-sized scratch value supports null and exactly aliased buffers
  // without modifying const input or allocating a transfer-sized buffer.
  for (size_t i = 0; i < len; ++i) {
    const uint8_t out = tx == nullptr ? 0 : tx[i];
    uint8_t in;
    if (spi_write_read_blocking(bus_, &out, &in, 1) != 1) return false;
    if (rx != nullptr) rx[i] = in;
  }
  return true;
}
bool PicoSpiSlaveDevice::transfer(const uint8_t* tx, uint8_t* rx,
                                  size_t len) const {
  if (len == 0 || (tx == nullptr && rx == nullptr)) return false;
  Transaction scope(*this);
  const bool success = scope.transfer(tx, rx, len);
  return scope.close() && success;
}
}  // namespace roo_io
#endif
