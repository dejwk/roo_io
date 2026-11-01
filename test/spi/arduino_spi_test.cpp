#include <Arduino.h>

#include "gtest/gtest.h"
#include "roo_io/spi/spi.h"

SPIClass SPI;
std::vector<std::pair<int, int>> pin_writes;
int configured_pin = -1;

// Verifies CS framing, SPI mode mapping, MSB order, and independent RX bytes.
TEST(ArduinoSpi, FramingAndSettings) {
  SPIClass spi;
  roo_io::SpiSlaveDevice device(spi, 5, 1000000, 1);
  uint8_t tx[] = {1, 2, 3};
  uint8_t rx[3];
  EXPECT_FALSE(device.transfer(tx, rx, 3));
  ASSERT_TRUE(device.init());
  ASSERT_TRUE(device.init());
  EXPECT_EQ(5, configured_pin);
  pin_writes.clear();
  ASSERT_TRUE(device.transfer(tx, rx, 3));
  EXPECT_EQ((std::vector<std::pair<int, int>>{{5, LOW}, {5, HIGH}}),
            pin_writes);
  EXPECT_EQ((std::vector<uint8_t>{1, 2, 3}), spi.sent);
  EXPECT_EQ(0xfe, rx[0]);
  EXPECT_EQ(0xfc, rx[2]);
  EXPECT_EQ(SPI_MODE1, spi.settings.mode);
  EXPECT_EQ(MSBFIRST, spi.settings.order);
  EXPECT_EQ(1000000u, spi.settings.frequency);
  EXPECT_EQ(1, spi.begins);
  EXPECT_EQ(1, spi.ends);
}

// Verifies receive-only zeros, transmit-only discard, aliasing, and bad inputs.
TEST(ArduinoSpi, BufferContracts) {
  SPIClass spi;
  roo_io::SpiSlaveDevice device(spi, 6);
  ASSERT_TRUE(device.init());
  uint8_t data[] = {0x55, 0xaa};
  ASSERT_TRUE(device.transfer(data, data, 2));
  EXPECT_EQ(0xaa, data[0]);
  EXPECT_EQ(0x55, data[1]);
  ASSERT_TRUE(device.transfer(nullptr, data, 2));
  EXPECT_EQ(0xff, data[0]);
  EXPECT_EQ(0xff, data[1]);
  ASSERT_TRUE(device.transfer(data, nullptr, 2));
  EXPECT_FALSE(device.transfer(nullptr, nullptr, 2));
  EXPECT_FALSE(device.transfer(data, data, 0));
  EXPECT_FALSE(roo_io::SpiSlaveDevice(spi, -1).init());
  EXPECT_FALSE(roo_io::SpiSlaveDevice(spi, 5, 0).init());
  EXPECT_FALSE(roo_io::SpiSlaveDevice(spi, 5, 1000, 4).init());
}

// Verifies multiple buffers share one CS assertion/settings reservation,
// ownership moves safely, and nested use of the same device is rejected.
TEST(ArduinoSpi, ScopedTransactions) {
  SPIClass spi;
  roo_io::SpiSlaveDevice device(spi, 5);
  ASSERT_TRUE(device.init());
  pin_writes.clear();
  uint8_t bytes[] = {0x12, 0x34};
  {
    roo_io::SpiTransaction scope = device.transaction();
    ASSERT_TRUE(scope.isOpen());
    EXPECT_FALSE(device.transaction().isOpen());
    EXPECT_FALSE(device.transfer(bytes, nullptr, 2));
    ASSERT_TRUE(scope.transfer(bytes, nullptr, 2));
    ASSERT_TRUE(scope.transfer(bytes, nullptr, 2));
    EXPECT_FALSE(scope.transfer(nullptr, nullptr, 2));
    roo_io::SpiTransaction moved(std::move(scope));
    EXPECT_FALSE(scope.isOpen());
    ASSERT_TRUE(moved.isOpen());
    EXPECT_EQ(1, spi.begins);
    EXPECT_EQ(0, spi.ends);
    EXPECT_EQ((std::vector<std::pair<int, int>>{{5, LOW}}), pin_writes);
  }
  EXPECT_EQ(1, spi.ends);
  EXPECT_EQ((std::vector<std::pair<int, int>>{{5, LOW}, {5, HIGH}}),
            pin_writes);
  {
    roo_io::SpiTransaction scope = device.transaction();
    ASSERT_TRUE(scope.isOpen());
    EXPECT_TRUE(scope.close());
    EXPECT_TRUE(scope.close());
    EXPECT_FALSE(scope.transfer(bytes, nullptr, 2));
  }
  EXPECT_EQ(2, spi.begins);
  EXPECT_EQ(2, spi.ends);
}

// Verifies existing Arduino device copies retain their bus/configuration while
// each scope releases its own reservation before the other copy is used.
TEST(ArduinoSpi, BorrowedDeviceCopy) {
  SPIClass spi;
  roo_io::SpiSlaveDevice device(spi, 5, 2000000, 3);
  ASSERT_TRUE(device.init());
  roo_io::SpiSlaveDevice copy(device);
  uint8_t byte = 0;
  EXPECT_TRUE(copy.transfer(&byte, nullptr, 1));
  EXPECT_TRUE(device.transfer(&byte, nullptr, 1));
  EXPECT_EQ(2, spi.begins);
  EXPECT_EQ(2, spi.ends);
  EXPECT_EQ(2000000u, spi.settings.frequency);
  EXPECT_EQ(SPI_MODE3, spi.settings.mode);
}
