#include <limits.h>
#include <stdint.h>

#include <string>
#include <utility>
#include <vector>

#include "driver/gpio.h"
#include "gtest/gtest.h"
#include "roo_io/spi/spi.h"

namespace {
esp_err_t add_result = ESP_OK;
esp_err_t transfer_result = ESP_OK;
esp_err_t acquire_result = ESP_OK;
bool direction_error = false;
int failed_level = -1;
int adds = 0;
int removes = 0;
int transfers = 0;
int acquires = 0;
int releases = 0;
spi_device_interface_config_t config;
spi_transaction_t transaction;
std::vector<std::string> events;
int device_token;

class IdfSpi : public testing::Test {
 protected:
  void SetUp() override {
    add_result = transfer_result = acquire_result = ESP_OK;
    direction_error = false;
    failed_level = -1;
    adds = removes = transfers = acquires = releases = 0;
    events.clear();
  }
};
}  // namespace
extern "C" {
esp_err_t gpio_set_level(gpio_num_t pin, uint32_t level) {
  EXPECT_EQ(5, pin);
  events.push_back(level == 0 ? "low" : "high");
  return static_cast<int>(level) == failed_level ? ESP_FAIL : ESP_OK;
}
esp_err_t gpio_set_direction(gpio_num_t pin, gpio_mode_t mode) {
  EXPECT_EQ(5, pin);
  EXPECT_EQ(GPIO_MODE_OUTPUT, mode);
  return direction_error ? ESP_FAIL : ESP_OK;
}
esp_err_t spi_bus_add_device(spi_host_device_t host,
                             const spi_device_interface_config_t* c,
                             spi_device_handle_t* out) {
  EXPECT_EQ(SPI2_HOST, host);
  if (add_result != ESP_OK) return add_result;
  config = *c;
  ++adds;
  *out = reinterpret_cast<spi_device_handle_t>(&device_token);
  return ESP_OK;
}
esp_err_t spi_bus_remove_device(spi_device_handle_t) {
  ++removes;
  return ESP_OK;
}
esp_err_t spi_device_acquire_bus(spi_device_handle_t, uint32_t) {
  ++acquires;
  events.push_back("acquire");
  return acquire_result;
}
void spi_device_release_bus(spi_device_handle_t) {
  ++releases;
  events.push_back("release");
}
esp_err_t spi_device_polling_transmit(spi_device_handle_t,
                                      spi_transaction_t* t) {
  ++transfers;
  transaction = *t;
  events.push_back("transfer");
  return transfer_result;
}
}

// Verifies registration retry/idempotence, manual CS configuration, and RAII.
TEST_F(IdfSpi, Lifecycle) {
  {
    roo_io::SpiSlaveDevice device(roo_io::SpiMasterBusHandle(SPI2_HOST), 5,
                                  1000000, 1);
    uint8_t byte = 0;
    EXPECT_FALSE(device.transfer(&byte, nullptr, 1));
    add_result = ESP_FAIL;
    EXPECT_FALSE(device.init());
    add_result = ESP_OK;
    ASSERT_TRUE(device.init());
    ASSERT_TRUE(device.init());
    EXPECT_EQ(1, adds);
    EXPECT_EQ(0, removes);
    EXPECT_EQ(1, config.mode);
    EXPECT_EQ(-1, config.spics_io_num);
    EXPECT_EQ(1000000, config.clock_speed_hz);
    EXPECT_EQ(1, config.queue_size);
  }
  EXPECT_EQ(1, removes);
}

// Verifies bit counts, null buffers, error propagation and overflow rejection.
TEST_F(IdfSpi, TransfersAndValidation) {
  roo_io::SpiSlaveDevice device(roo_io::SpiMasterBusHandle(), 5);
  ASSERT_TRUE(device.init());
  uint8_t tx[] = {1, 2, 3};
  uint8_t rx[3];
  ASSERT_TRUE(device.transfer(tx, rx, 3));
  EXPECT_EQ(24u, transaction.length);
  EXPECT_EQ(tx, transaction.tx_buffer);
  EXPECT_EQ(rx, transaction.rx_buffer);
  ASSERT_TRUE(device.transfer(nullptr, rx, 3));
  ASSERT_TRUE(device.transfer(tx, nullptr, 3));
  transfer_result = ESP_FAIL;
  EXPECT_FALSE(device.transfer(tx, rx, 3));
  EXPECT_EQ(acquires, releases);
  transfer_result = ESP_OK;
  const int before = transfers;
  EXPECT_FALSE(device.transfer(tx, rx, SIZE_MAX));
  EXPECT_FALSE(device.transfer(tx, rx, 0));
  EXPECT_FALSE(device.transfer(nullptr, nullptr, 3));
  EXPECT_EQ(before, transfers);
  EXPECT_FALSE(roo_io::SpiSlaveDevice(roo_io::SpiMasterBusHandle(), -1).init());
  EXPECT_FALSE(
      roo_io::SpiSlaveDevice(roo_io::SpiMasterBusHandle(), 5, 0).init());
  EXPECT_FALSE(
      roo_io::SpiSlaveDevice(roo_io::SpiMasterBusHandle(), 5, UINT32_MAX)
          .init());
  EXPECT_FALSE(
      roo_io::SpiSlaveDevice(roo_io::SpiMasterBusHandle(), 5, 1000, 4).init());
}

// Verifies a fragmented exchange uses one lock/CS pair and move ownership;
// nested same-device scopes fail without trying to acquire the bus again.
TEST_F(IdfSpi, ScopedTransactions) {
  roo_io::SpiSlaveDevice device(roo_io::SpiMasterBusHandle(), 5);
  ASSERT_TRUE(device.init());
  events.clear();
  uint8_t byte = 1;
  {
    roo_io::SpiTransaction scope = device.transaction();
    ASSERT_TRUE(scope.isOpen());
    EXPECT_FALSE(device.transaction().isOpen());
    EXPECT_FALSE(device.transfer(&byte, nullptr, 1));
    ASSERT_TRUE(scope.transfer(&byte, nullptr, 1));
    ASSERT_TRUE(scope.transfer(&byte, nullptr, 1));
    roo_io::SpiTransaction moved(std::move(scope));
    EXPECT_FALSE(scope.isOpen());
    EXPECT_TRUE(moved.isOpen());
  }
  EXPECT_EQ((std::vector<std::string>{"acquire", "low", "transfer", "transfer",
                                      "high", "release"}),
            events);
  EXPECT_EQ(1, acquires);
  EXPECT_EQ(1, releases);
}

// Verifies failed acquisition and CS assertion always leave the bus available.
TEST_F(IdfSpi, AcquisitionFailure) {
  roo_io::SpiSlaveDevice device(roo_io::SpiMasterBusHandle(), 5);
  ASSERT_TRUE(device.init());
  events.clear();
  acquire_result = ESP_FAIL;
  EXPECT_FALSE(device.transaction().isOpen());
  EXPECT_EQ((std::vector<std::string>{"acquire"}), events);
  EXPECT_EQ(0, releases);
  acquire_result = ESP_OK;
  failed_level = 0;
  events.clear();
  EXPECT_FALSE(device.transaction().isOpen());
  EXPECT_EQ((std::vector<std::string>{"acquire", "low", "release"}), events);
  failed_level = -1;
  EXPECT_TRUE(device.transaction().isOpen());
  EXPECT_EQ(2, releases);
}

// Verifies transfer failure permits a protocol cleanup attempt, close reports
// GPIO failure, and the lock is released even when deassertion fails.
TEST_F(IdfSpi, TransferAndCloseFailure) {
  roo_io::SpiSlaveDevice device(roo_io::SpiMasterBusHandle(), 5);
  ASSERT_TRUE(device.init());
  uint8_t byte = 0;
  roo_io::SpiTransaction scope = device.transaction();
  ASSERT_TRUE(scope.isOpen());
  transfer_result = ESP_FAIL;
  EXPECT_FALSE(scope.transfer(&byte, nullptr, 1));
  transfer_result = ESP_OK;
  EXPECT_TRUE(scope.transfer(&byte, nullptr, 1));
  failed_level = 1;
  EXPECT_FALSE(scope.close());
  EXPECT_TRUE(scope.close());
  EXPECT_FALSE(scope.isOpen());
  EXPECT_EQ(1, releases);
  EXPECT_FALSE(device.transfer(&byte, nullptr, 1));
  EXPECT_EQ(2, releases);
  failed_level = -1;
  EXPECT_TRUE(device.transfer(&byte, nullptr, 1));
  EXPECT_EQ(3, releases);
}

// Verifies invalid CS setup never registers a device and can be retried.
TEST_F(IdfSpi, GpioSetupFailure) {
  roo_io::SpiSlaveDevice device(roo_io::SpiMasterBusHandle(), 5);
  direction_error = true;
  EXPECT_FALSE(device.init());
  EXPECT_EQ(0, adds);
  direction_error = false;
  failed_level = 1;
  EXPECT_FALSE(device.init());
  EXPECT_EQ(0, adds);
  failed_level = -1;
  EXPECT_TRUE(device.init());
  EXPECT_EQ(1, adds);
}
