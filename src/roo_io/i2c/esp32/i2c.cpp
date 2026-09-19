#include "roo_io/i2c/esp32/i2c.h"

#if defined(ESP_PLATFORM) && !defined(ARDUINO)
#include <utility>
namespace roo_io {
struct Esp32I2cMasterBusHandle::State {
  i2c_port_num_t port = 0;
  i2c_master_bus_handle_t handle = nullptr;
  uint32_t frequency = 100000;
  bool owned = false;
  ~State() {
    if (owned && handle != nullptr) {
      i2c_del_master_bus(handle);
    }
  }
};

Esp32I2cMasterBusHandle::Esp32I2cMasterBusHandle(i2c_port_num_t port)
    : state_(std::make_shared<State>()) {
  state_->port = port;
}

Esp32I2cMasterBusHandle::Esp32I2cMasterBusHandle(i2c_master_bus_handle_t handle,
                                                 uint32_t frequency)
    : borrowed_handle_(handle),
      borrowed_frequency_(frequency == 0 ? 100000 : frequency) {}

bool Esp32I2cMasterBusHandle::init(int sda, int scl, uint32_t frequency) {
  if (state_ == nullptr || state_->handle != nullptr) {
    return false;
  }
  i2c_master_bus_config_t config = {};
  config.i2c_port = state_->port;
  config.sda_io_num = static_cast<gpio_num_t>(sda);
  config.scl_io_num = static_cast<gpio_num_t>(scl);
  config.clk_source = I2C_CLK_SRC_DEFAULT;
  config.glitch_ignore_cnt = 7;
  config.flags.enable_internal_pullup = true;
  i2c_master_bus_handle_t handle = nullptr;
  if (i2c_new_master_bus(&config, &handle) != ESP_OK) {
    return false;
  }
  state_->handle = handle;
  state_->owned = true;
  state_->frequency = frequency == 0 ? 100000 : frequency;
  return true;
}

uint32_t Esp32I2cMasterBusHandle::frequency() const {
  return state_ != nullptr ? state_->frequency : borrowed_frequency_;
}

i2c_master_bus_handle_t Esp32I2cMasterBusHandle::resolve() const {
  if (state_ == nullptr) {
    return borrowed_handle_;
  }
  if (state_->handle == nullptr) {
    i2c_master_bus_handle_t handle = nullptr;
    if (i2c_master_get_bus_handle(state_->port, &handle) != ESP_OK) {
      return nullptr;
    }
    state_->handle = handle;
  }
  return state_->handle;
}

Esp32I2cSlaveDevice::Esp32I2cSlaveDevice(Esp32I2cMasterBusHandle bus,
                                         uint8_t address, int timeout_ms)
    : bus_(std::move(bus)), address_(address), timeout_ms_(timeout_ms) {}

Esp32I2cSlaveDevice::~Esp32I2cSlaveDevice() {
  if (device_ != nullptr) {
    i2c_master_bus_rm_device(device_);
  }
}

bool Esp32I2cSlaveDevice::init() {
  if (device_ != nullptr) {
    return true;
  }
  if (address_ > 0x7f || timeout_ms_ < 0) {
    return false;
  }
  const i2c_master_bus_handle_t handle = bus_.resolve();
  if (handle == nullptr) {
    return false;
  }
  i2c_device_config_t config = {};
  config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  config.device_address = address_;
  config.scl_speed_hz = bus_.frequency();
  i2c_master_dev_handle_t device = nullptr;
  if (i2c_master_bus_add_device(handle, &config, &device) != ESP_OK) {
    return false;
  }
  device_ = device;
  return true;
}

bool Esp32I2cSlaveDevice::transmit(const roo::byte* data, size_t len) const {
  return device_ != nullptr && data != nullptr && len > 0 &&
         i2c_master_transmit(device_, reinterpret_cast<const uint8_t*>(data),
                             len, timeout_ms_) == ESP_OK;
}

size_t Esp32I2cSlaveDevice::receive(roo::byte* data, size_t len) const {
  if (device_ == nullptr || data == nullptr || len == 0) {
    return 0;
  }
  return i2c_master_receive(device_, reinterpret_cast<uint8_t*>(data), len,
                            timeout_ms_) == ESP_OK
             ? len
             : 0;
}

bool Esp32I2cSlaveDevice::transmitReceive(const roo::byte* tx, size_t tx_len,
                                          roo::byte* rx, size_t rx_len) const {
  return device_ != nullptr && tx != nullptr && rx != nullptr && tx_len > 0 &&
         rx_len > 0 &&
         i2c_master_transmit_receive(
             device_, reinterpret_cast<const uint8_t*>(tx), tx_len,
             reinterpret_cast<uint8_t*>(rx), rx_len, timeout_ms_) == ESP_OK;
}
}  // namespace roo_io
#endif
