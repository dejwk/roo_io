#pragma once
#include <vector>

#include "driver/i2c_master.h"

// Scripted native API boundary for fault injection, independent of the
// emulator.
namespace i2c_test {
/// Observable calls and scripted outcomes for adapter and RTC fault tests.
struct State {
  esp_err_t create_result = ESP_OK;
  esp_err_t add_result = ESP_OK;
  esp_err_t transfer_result = ESP_OK;
  bool available = false;
  int created = 0;
  int deleted = 0;
  int added = 0;
  int removed = 0;
  int combined = 0;
  int timeout = 0;
  uint16_t address = 0;
  uint32_t frequency = 0;
  std::vector<uint8_t> sent;
  std::vector<uint8_t> received = {0x56, 0x34, 0x12, 3, 0x15, 0x06, 0x24};
};
/// Returns the process-local scripted state.
State& state();

/// Returns a stable opaque bus handle for this scripted boundary.
i2c_master_bus_handle_t handle();
}  // namespace i2c_test
