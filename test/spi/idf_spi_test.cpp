#include <limits.h>
#include <stdint.h>

#include <vector>

#include "gtest/gtest.h"
#include "roo_io/spi/spi.h"

namespace {
esp_err_t add_result = ESP_OK;
esp_err_t transfer_result = ESP_OK;
int adds = 0;
int removes = 0;
int transfers = 0;
spi_device_interface_config_t config;
spi_transaction_t transaction;
int device_token;
}  // namespace
extern "C" {
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
esp_err_t spi_device_polling_transmit(spi_device_handle_t,
                                      spi_transaction_t* t) {
  ++transfers;
  transaction = *t;
  return transfer_result;
}
}

// Verifies registration retry/idempotence, configured mode/CS, and RAII
// removal.
TEST(IdfSpi, Lifecycle) {
  adds = removes = 0;
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
    EXPECT_EQ(5, config.spics_io_num);
    EXPECT_EQ(1000000, config.clock_speed_hz);
    EXPECT_EQ(1, config.queue_size);
  }
  EXPECT_EQ(1, removes);
}

// Verifies bit counts, null buffers, error propagation, and overflow rejection.
TEST(IdfSpi, TransfersAndValidation) {
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
