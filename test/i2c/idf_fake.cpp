#include "idf_fake.h"

#include <algorithm>

#include "gtest/gtest.h"

#if defined(ARDUINO)
#error This boundary must compile without Arduino.
#endif
struct i2c_master_bus_t {};

struct i2c_master_dev_t {};
namespace i2c_test {
State& state() {
  static State s;
  return s;
}
i2c_master_bus_handle_t handle() {
  static i2c_master_bus_t bus;
  return &bus;
}
}  // namespace i2c_test
using i2c_test::state;
extern "C" {
esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t*,
                             i2c_master_bus_handle_t* result) {
  if (state().create_result != ESP_OK) {
    return state().create_result;
  }
  state().available = true;
  ++state().created;
  *result = i2c_test::handle();
  return ESP_OK;
}
esp_err_t i2c_master_get_bus_handle(i2c_port_num_t,
                                    i2c_master_bus_handle_t* result) {
  if (!state().available) {
    return ESP_ERR_NOT_FOUND;
  }
  *result = i2c_test::handle();
  return ESP_OK;
}
esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t bus) {
  EXPECT_EQ(bus, i2c_test::handle());
  EXPECT_EQ(state().added, state().removed);
  ++state().deleted;
  state().available = false;
  return ESP_OK;
}
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus,
                                    const i2c_device_config_t* config,
                                    i2c_master_dev_handle_t* result) {
  EXPECT_EQ(bus, i2c_test::handle());
  if (state().add_result != ESP_OK) {
    return state().add_result;
  }
  state().address = config->device_address;
  state().frequency = config->scl_speed_hz;
  ++state().added;
  *result = new i2c_master_dev_t;
  return ESP_OK;
}
esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t device) {
  ++state().removed;
  delete device;
  return ESP_OK;
}
esp_err_t i2c_master_transmit(i2c_master_dev_handle_t, const uint8_t* data,
                              size_t len, int timeout) {
  state().timeout = timeout;
  state().sent.assign(data, data + len);
  return state().transfer_result;
}
esp_err_t i2c_master_receive(i2c_master_dev_handle_t, uint8_t* data, size_t len,
                             int timeout) {
  state().timeout = timeout;
  if (state().transfer_result != ESP_OK) {
    return state().transfer_result;
  }
  if (len > state().received.size()) {
    return ESP_ERR_INVALID_SIZE;
  }
  std::copy_n(state().received.begin(), len, data);
  return ESP_OK;
}
esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t device,
                                      const uint8_t* tx, size_t tx_len,
                                      uint8_t* rx, size_t rx_len, int timeout) {
  ++state().combined;
  const esp_err_t result = i2c_master_transmit(device, tx, tx_len, timeout);
  return result == ESP_OK ? i2c_master_receive(device, rx, rx_len, timeout)
                          : result;
}
}
